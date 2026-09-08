#!/usr/bin/env node

// Exercise the actual HUD consumers with the byte layout emitted by master's
// server. Rendering and localization are stubbed; the reader and handlers are not.
const fs = require("fs");
const os = require("os");
const path = require("path");
const { spawnSync } = require("child_process");

const root = path.resolve(__dirname, "..");
function extractFunction(relativePath, signature) {
  const source = fs.readFileSync(path.join(root, relativePath), "utf8");
  const start = source.indexOf(signature);
  if (start < 0) throw new Error(`Missing function: ${signature}`);
  const opening = source.indexOf("{", start);
  let depth = 1;
  let end = opening + 1;
  for (; end < source.length && depth; ++end) {
    if (source[end] === "{") ++depth;
    if (source[end] === "}") --depth;
  }
  if (depth) throw new Error(`Unbalanced function: ${signature}`);
  return source.slice(start, end);
}

const handlers = [
  ["src/game/client/hud/text_message.cpp", "void StripEndNewlineFromString("],
  ["src/game/client/hud/text_message.cpp", "char* ConvertCRtoNL("],
  ["src/game/client/hud/text_message.cpp", "int CHudTextMessage::MsgFunc_TextMsg("],
  ["src/game/client/hud/scoreboard.cpp", "int CHudScoreboard::MsgFunc_TeamScore("],
  ["src/game/client/hud/ammo.cpp", "int CHudAmmo::MsgFunc_Brass("],
].map(([file, signature]) => extractFunction(file, signature)).join("\n");
const brassProducer = extractFunction("src/game/server/combat/weapons.cpp", "void EjectBrass(");

const harness = String.raw`
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <strings.h>
#include <vector>
#include <initializer_list>

static int readerErrors;
static void ReaderDiagnostic(const char *, ...) { ++readerErrors; }
struct Engine { void (*Con_DPrintf)(const char *, ...); } gEngfuncs{ReaderDiagnostic};
#include "game/client/runtime/parsemsg.h"

#define stricmp strcasecmp
#define TRUE 1
#define MAX_TEXTMSG_STRING 256
enum { HUD_PRINTNOTIFY = 1, HUD_PRINTCONSOLE, HUD_PRINTTALK, HUD_PRINTCENTER, HUD_PRINTRADIO };
constexpr int MAX_PLAYERS = 33;
static std::string output;
void CenterPrint(char *text) { output = text; }
void ConsolePrint(char *text) { output = text; }
struct PlayerInfo { const char *name = nullptr; } g_PlayerInfoList[MAX_PLAYERS];
struct TeamInfo { char name[16]{}; int frags = 0; int deaths = 0; bool scores_overriden = false; } g_TeamInfo[4];
struct Cvar { float value = 0; } rightHand;
struct Hud {
  Cvar *cl_righthand = &rightHand;
  struct ShowWin {
    bool OnTextMessage(const char *text) { return !std::strcmp(text, "#CTs_Win"); }
  } m_ShowWin;
  struct SayText {
    void SayTextPrint(char *text, int, int = -1) { output = text; }
  } m_SayText;
} gHUD;
class CHudTextMessage {
public:
  char *LookupString(const char *text, int * = nullptr) { return const_cast<char *>(text); }
  int MsgFunc_TextMsg(const char *, int, void *);
};
class CHudScoreboard {
public:
  int m_iNumTeams = 0;
  int m_iTeamScore_T = 0;
  int m_iTeamScore_CT = 0;
  int MsgFunc_TeamScore(const char *, int, void *);
};
struct Vector { float x, y, z; };
struct BrassResult { Vector origin{}, velocity{}; float rotation = 0, life = 0; int model = 0, sound = 0, count = 0; } brassResult;
static int checkedClient;
bool EV_IsLocal(int client) { checkedClient = client; return client == 1; }
void EV_EjectBrass(Vector origin, Vector velocity, float rotation, int model, int sound, float life) {
  brassResult = {origin, velocity, rotation, life, model, sound, brassResult.count + 1};
}
void sincosf(float angle, float *sine, float *cosine) { *sine = std::sin(angle); *cosine = std::cos(angle); }
class CHudAmmo {
public:
  int MsgFunc_Brass(const char *, int, void *);
};
` + handlers + String.raw`

static bool g_bIsCzeroGame;
static std::vector<unsigned char> brassWire;
void WriteBrassShort(int value) {
  brassWire.push_back(value & 255);
  brassWire.push_back((value >> 8) & 255);
}
#define TE_MODEL 106
#define MESSAGE_BEGIN(...) brassWire.clear()
#define MESSAGE_END() ((void)0)
#define WRITE_BYTE(value) brassWire.push_back((value) & 255)
#define WRITE_SHORT(value) WriteBrassShort(value)
#define WRITE_COORD(value) WriteBrassShort(static_cast<int>((value) * 8))
#define WRITE_ANGLE(value) WRITE_BYTE(static_cast<int>((value) * 256 / 360))
` + brassProducer + String.raw`

void WriteString(std::vector<unsigned char> &wire, const std::string &text) {
  wire.insert(wire.end(), text.begin(), text.end());
  wire.push_back(0);
}
void CheckText(const std::string &format, std::initializer_list<std::string> parameters, const std::string &expected) {
  std::vector<unsigned char> wire{HUD_PRINTCONSOLE};
  WriteString(wire, format);
  for (const auto &parameter : parameters) WriteString(wire, parameter);
  CHudTextMessage hud;
  output.clear();
  const int errors = readerErrors;
  assert(hud.MsgFunc_TextMsg("TextMsg", wire.size(), wire.data()) == 1);
  assert(readerErrors == errors);
  assert(output == expected);
}
void SendScore(CHudScoreboard &hud, const char *team, int score) {
  std::vector<unsigned char> wire;
  WriteString(wire, team);
  wire.push_back(score & 255);
  wire.push_back((score >> 8) & 255);
  const int errors = readerErrors;
  assert(hud.MsgFunc_TeamScore("TeamScore", wire.size(), wire.data()) == 1);
  assert(readerErrors == errors);
}
void CheckBrass(bool czero, bool local, bool rightHanded, float rotation) {
  const Vector origin{20.5f, -30.25f, 64.0f};
  const Vector left{100.0f, 200.0f, 300.0f};
  const Vector velocity{11.5f, -22.25f, 33.75f};
  g_bIsCzeroGame = czero;
  rightHand.value = rightHanded ? 1 : 0;
  EjectBrass(origin, left, velocity, rotation, 321, 2, local ? 1 : 3);
  assert(brassWire.size() == (czero ? 17 : 25));
  const int errors = readerErrors, emitted = brassResult.count;
  checkedClient = 0;
  CHudAmmo hud;
  assert(hud.MsgFunc_Brass("Brass", brassWire.size(), brassWire.data()) == 1);
  assert(readerErrors == errors && brassResult.count == emitted + 1);
  if (rightHanded) assert(checkedClient == (local ? 1 : 3));
  assert(brassResult.model == 321 && brassResult.sound == 2);
  assert(std::fabs(brassResult.life - 2.5f) < 0.001f);
  assert(std::fabs(brassResult.rotation - rotation) < 0.001f);
  const float sine = std::sin(std::fabs(rotation * M_PI / 180.0f));
  const float cosine = std::cos(std::fabs(rotation * M_PI / 180.0f));
  const bool mirrored = local && rightHanded;
  assert(std::fabs(brassResult.origin.x - (origin.x + (mirrored ? 9 : -9) * sine)) < 0.001f);
  assert(std::fabs(brassResult.origin.y - (origin.y + (mirrored ? -9 : 9) * cosine)) < 0.001f);
  assert(brassResult.origin.z == origin.z);
  assert(std::fabs(brassResult.velocity.x - (velocity.x + (mirrored ? -120 * sine : 0))) < 0.001f);
  assert(std::fabs(brassResult.velocity.y - (velocity.y + (mirrored ? 120 * cosine : 0))) < 0.001f);
  assert(brassResult.velocity.z == velocity.z);
}
int main() {
  CheckText("No parameters", {}, "No parameters");
  CheckText("Welcome %s1", {"Alice\n"}, "Welcome Alice");
  CheckText("%s1 + %s2", {"A", "B\r"}, "A + B");
  CheckText("%s1/%s2/%s3", {"A", "B", "C"}, "A/B/C");
  CheckText("%s1/%s2/%s3/%s4", {"A", "B", "C", "D"}, "A/B/C/D");
  CheckText("%s1/%s2/%s3/%s4", {""}, "///");
  CheckText("%s1/%s2/%s3/%s4", {}, "///");
  CheckText("[%s1]", {""}, "[]");
  CheckText("Literal 100%%", {}, "Literal 100%");
  CheckText("%s1", {std::string(400, 'x')}, std::string(255, 'x'));
  CheckText(std::string(400, 'x'), {}, std::string(255, 'x'));

  char empty[] = "";
  StripEndNewlineFromString(empty);
  assert(empty[0] == 0);

  CHudScoreboard hud;
  SendScore(hud, "CT", 7); // Initial TeamScore before TeamInfo.
  SendScore(hud, "TERRORIST", 5);
  assert(hud.m_iTeamScore_CT == 7 && hud.m_iTeamScore_T == 5);
  hud.m_iNumTeams = 2;
  std::strcpy(g_TeamInfo[1].name, "CT");
  std::strcpy(g_TeamInfo[2].name, "TERRORIST");
  g_TeamInfo[1].deaths = 9;
  SendScore(hud, "CT", 12);
  SendScore(hud, "TERRORIST", 255);
  assert(hud.m_iTeamScore_CT == 12 && hud.m_iTeamScore_T == 255);
  assert(g_TeamInfo[1].frags == 12 && g_TeamInfo[2].frags == 255);
  assert(g_TeamInfo[1].deaths == 9); // Deaths are not part of this message.

  unsigned char malformed[] = {'C', 'T', 0, 1};
  hud.MsgFunc_TeamScore("TeamScore", sizeof(malformed), malformed);
  assert(readerErrors == 1); // Truncated required fields remain diagnosable.
  for (bool czero : {false, true})
    for (bool local : {false, true})
      for (bool right : {false, true})
        for (float rotation : {0.0f, 90.0f, -90.0f})
          CheckBrass(czero, local, right, rotation);
  const int errors = readerErrors, emitted = brassResult.count;
  CHudAmmo ammo;
  assert(ammo.MsgFunc_Brass("Brass", brassWire.size() - 1, brassWire.data()) == 0);
  assert(readerErrors == errors + 1 && brassResult.count == emitted);
  std::puts("HUD messages: 11 TextMsg, 4 TeamScore, 24 Brass producer/consumer cases and malformed-packet diagnostics passed");
}
`;

const temporary = fs.mkdtempSync(path.join(os.tmpdir(), "csmoe-hud-messages-"));
const executable = path.join(temporary, "hud-messages-test");
try {
  const compiled = spawnSync("c++", ["-std=c++17", "-fsanitize=address", "-fno-omit-frame-pointer", "-I", path.join(root, "src"), "-I", path.join(root, "vendor"), "-x", "c++", "-", "-o", executable], {
    input: harness, encoding: "utf8",
  });
  if (compiled.status !== 0) {
    process.stderr.write(compiled.stderr || String(compiled.error));
    process.exitCode = 1;
  } else {
    const tested = spawnSync(executable, [], { stdio: "inherit" });
    process.exitCode = tested.status === 0 ? 0 : 1;
  }
} finally {
  if (fs.existsSync(executable)) fs.unlinkSync(executable);
  fs.rmdirSync(temporary);
}
