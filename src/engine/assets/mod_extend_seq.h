#ifndef MOD_EXTEND_SEQ_H
#define MOD_EXTEND_SEQ_H

#include "engine/runtime/common.h"

/* Replaces and frees buffer only after all available packs have been validated. */
byte *Mod_LoadExtendSeq( const char *name, byte *buffer, fs_offset_t *filesize );

#endif
