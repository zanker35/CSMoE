/*
mod_extend_seq.c - optional CSO studio animation packs

A pack contains two little-endian ints (sequence count, descriptor offset),
then packed events/pivots, animation data, and mstudioseqdesc_t descriptors.
The descriptors retain offsets from the original unsplit model. Only animation
offset differences are meaningful; event and pivot offsets must be rebuilt.
*/

#include "mod_extend_seq.h"
#include "mod_local.h"
#include "studio.h"
#include <limits.h>

#define MAX_SEQUENCEPACKS 256

typedef struct
{
	int numseq;
	int seqindex;
} studioseqpack_t;

static qboolean Mod_SeqRange( int offset, int count, size_t itemsize, size_t size )
{
	return offset >= 0 && count >= 0 && (size_t)offset <= size &&
		(size_t)count <= (size - (size_t)offset) / itemsize;
}

static qboolean Mod_SeqAnimationValid( const byte *data, int end, int offset,
	int numbones, const mstudioseqdesc_t *seq )
{
	int i, axis, numblends = LittleLong( seq->numblends );
	int numframes = LittleLong( seq->numframes );
	int count;

	if( numblends < 1 || numblends > MAXSTUDIOBLENDS || numframes < 1 ||
		numframes > MAXSTUDIOANIMATIONS || LittleLong( seq->seqgroup ) != 0 )
		return false;
	count = numbones * numblends;
	if( (offset & 1) || !Mod_SeqRange( offset, count, sizeof( mstudioanim_t ), end ))
		return false;

	for( i = 0; i < count; i++ )
	{
		int boneoffset = offset + i * sizeof( mstudioanim_t );
		const mstudioanim_t *anim = (const mstudioanim_t *)(data + boneoffset);
		for( axis = 0; axis < 6; axis++ )
		{
			unsigned short relative = LittleShort( anim->offset[axis] );
			int frames = 0, pos;
			if( relative == 0 ) continue;
			if( (relative & 1) || relative > end - boneoffset ) return false;
			pos = boneoffset + relative;
			while( frames < numframes )
			{
				const mstudioanimvalue_t *value;
				if( !Mod_SeqRange( pos, 1, sizeof( mstudioanimvalue_t ), end ))
					return false;
				value = (const mstudioanimvalue_t *)(data + pos);
				if( value->num.valid == 0 || value->num.total < value->num.valid ||
					!Mod_SeqRange( pos, value->num.valid + 1, sizeof( *value ), end ))
					return false;
				frames += value->num.total;
				pos += (value->num.valid + 1) * sizeof( *value );
			}
		}
	}
	return true;
}

static qboolean Mod_SeqPackValid( byte *data, fs_offset_t filesize, int numbones )
{
	studioseqpack_t *pack = (studioseqpack_t *)data;
	mstudioseqdesc_t *seq;
	int i, count, end, metadata, firstanim;

	if( filesize < (fs_offset_t)sizeof( *pack ) || filesize > INT_MAX ) return false;
	count = LittleLong( pack->numseq );
	end = LittleLong( pack->seqindex );
	if( count < 1 || count > MAXSTUDIOSEQUENCES || end < (int)sizeof( *pack ) ||
		(end & 3) || !Mod_SeqRange( end, count, sizeof( *seq ), filesize ))
		return false;

	seq = (mstudioseqdesc_t *)(data + end);
	metadata = sizeof( *pack );
	for( i = 0; i < count; i++ )
	{
		int events = LittleLong( seq[i].numevents );
		int pivots = LittleLong( seq[i].numpivots );
		if( !memchr( seq[i].label, 0, sizeof( seq[i].label )) ||
			!Mod_SeqRange( metadata, events, sizeof( mstudioevent_t ), end ))
			return false;
		metadata += events * sizeof( mstudioevent_t );
		if( !Mod_SeqRange( metadata, pivots, sizeof( mstudiopivot_t ), end ))
			return false;
		metadata += pivots * sizeof( mstudiopivot_t );
		/* These unused movement tables have no representation in a pack. */
		if( LittleLong( seq[i].automoveposindex ) || LittleLong( seq[i].automoveangleindex ))
			return false;
	}

	firstanim = LittleLong( seq[0].animindex );
	if( firstanim < 0 ) return false;
	for( i = 0; i < count; i++ )
	{
		int original = LittleLong( seq[i].animindex );
		int offset;
		if( original < firstanim || original - firstanim > end - metadata )
			return false;
		offset = metadata + (original - firstanim);
		if( !Mod_SeqAnimationValid( data, end, offset, numbones, &seq[i] ))
			return false;
	}
	return true;
}

byte *Mod_LoadExtendSeq( const char *name, byte *buffer, fs_offset_t *filesize )
{
	studiohdr_t *header = (studiohdr_t *)buffer;
	byte *packs[MAX_SEQUENCEPACKS] = { 0 }, *merged;
	mstudioseqdesc_t *sequences;
	mstudiotexture_t *textures;
	char basename[MAX_SYSPATH], filename[MAX_SYSPATH];
	int i, j, numpacks = 0, length, prefix, numseq, seqindex, numtextures, textureindex;
	int numbones, packbytes = 0, total, newseqindex, newtexturedata, cursor, outputseq;
	fs_offset_t packsize;

	if( !name || !buffer || !filesize || *filesize < (fs_offset_t)sizeof( *header ) ||
		LittleLong( header->ident ) != IDSTUDIOHEADER ||
		LittleLong( header->version ) != STUDIO_VERSION ||
		Q_strlen( name ) >= (int)sizeof( basename ) - 8 )
		return buffer;

	Q_strncpy( basename, name, sizeof( basename ));
	FS_StripExtension( basename );
	Q_snprintf( filename, sizeof( filename ), "%s1.seq", basename );
	packs[0] = FS_LoadFile( filename, &packsize, false );
	if( !packs[0] ) return buffer;
	numpacks = 1;

	length = LittleLong( header->length );
	prefix = LittleLong( header->texturedataindex );
	numseq = LittleLong( header->numseq );
	seqindex = LittleLong( header->seqindex );
	numtextures = LittleLong( header->numtextures );
	textureindex = LittleLong( header->textureindex );
	numbones = LittleLong( header->numbones );
	/* Insert before raw textures: the studio cache discards everything after them. */
	if( length < (int)sizeof( *header ) || length > *filesize || prefix < (int)sizeof( *header ) ||
		prefix > length || (prefix & 3) || (seqindex & 3) || (textureindex & 3) ||
		numbones < 1 || numbones > MAXSTUDIOBONES || numseq < 1 ||
		!Mod_SeqRange( seqindex, numseq, sizeof( *sequences ), prefix ) ||
		!Mod_SeqRange( textureindex, numtextures, sizeof( *textures ), prefix ))
		goto invalid;

	textures = (mstudiotexture_t *)(buffer + textureindex);
	for( i = 0; i < numtextures; i++ )
	{
		int index = LittleLong( textures[i].index );
		if( index < prefix || index >= length ) goto invalid;
	}

	for( i = 0; i < MAX_SEQUENCEPACKS; i++ )
	{
		studioseqpack_t *pack;
		int count, bytes;
		if( i > 0 )
		{
			Q_snprintf( filename, sizeof( filename ), "%s%d.seq", basename, i + 1 );
			packs[i] = FS_LoadFile( filename, &packsize, false );
			if( !packs[i] ) break;
			numpacks++;
		}
		if( !Mod_SeqPackValid( packs[i], packsize, numbones )) goto invalid;
		pack = (studioseqpack_t *)packs[i];
		count = LittleLong( pack->numseq );
		bytes = LittleLong( pack->seqindex );
		if( numseq > INT_MAX / (int)sizeof( *sequences ) - count ||
			bytes > INT_MAX - packbytes ) goto invalid;
		numseq += count;
		packbytes += bytes;
	}
	if( packbytes > INT_MAX - length ||
		numseq > (INT_MAX - length - packbytes) / (int)sizeof( *sequences ))
		goto invalid;
	total = length + packbytes + numseq * sizeof( *sequences );
	newseqindex = prefix + packbytes;
	newtexturedata = newseqindex + numseq * sizeof( *sequences );
	merged = Mem_Alloc( com_studiocache, total );
	Q_memcpy( merged, buffer, prefix );
	Q_memcpy( merged + newtexturedata, buffer + prefix, length - prefix );
	sequences = (mstudioseqdesc_t *)(merged + newseqindex);
	outputseq = LittleLong( header->numseq );
	Q_memcpy( sequences, buffer + seqindex, outputseq * sizeof( *sequences ));
	cursor = prefix;
	for( i = 0; i < numpacks; i++ )
	{
		studioseqpack_t *pack = (studioseqpack_t *)packs[i];
		int count = LittleLong( pack->numseq ), bytes = LittleLong( pack->seqindex );
		mstudioseqdesc_t *source = (mstudioseqdesc_t *)(packs[i] + bytes);
		int metadata = sizeof( *pack ), firstanim = LittleLong( source[0].animindex );
		Q_memcpy( merged + cursor, packs[i], bytes );
		Q_memcpy( sequences + outputseq, source, count * sizeof( *source ));
		for( j = 0; j < count; j++ )
		{
			mstudioseqdesc_t *seq = &sequences[outputseq + j];
			seq->eventindex = LittleLong( cursor + metadata );
			metadata += LittleLong( seq->numevents ) * sizeof( mstudioevent_t );
			seq->pivotindex = LittleLong( cursor + metadata );
			metadata += LittleLong( seq->numpivots ) * sizeof( mstudiopivot_t );
		}
		for( j = 0; j < count; j++ )
			sequences[outputseq + j].animindex = LittleLong( cursor + metadata +
				(LittleLong( source[j].animindex ) - firstanim) );
		outputseq += count;
		cursor += bytes;
		Mem_Free( packs[i] );
	}

	header = (studiohdr_t *)merged;
	header->length = LittleLong( total );
	header->numseq = LittleLong( numseq );
	header->seqindex = LittleLong( newseqindex );
	header->texturedataindex = LittleLong( newtexturedata );
	textures = (mstudiotexture_t *)(merged + textureindex);
	for( i = 0; i < numtextures; i++ )
		textures[i].index = LittleLong( LittleLong( textures[i].index ) + (newtexturedata - prefix) );
	Mem_Free( buffer );
	*filesize = total;
	MsgDev( D_NOTE, "Mod_LoadExtendSeq: %s: %d packs, %d sequences\n", name, numpacks, numseq );
	return merged;

invalid:
	MsgDev( D_WARN, "Mod_LoadExtendSeq: ignoring invalid sequence pack for %s (%s)\n", name, filename );
	for( i = 0; i < numpacks; i++ ) Mem_Free( packs[i] );
	return buffer;
}
