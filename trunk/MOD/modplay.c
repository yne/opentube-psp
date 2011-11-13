/*
 * Copyright(c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modplay.h"
#include "s3m.h"
#include "mod.h"
#include "xm.h"

const MODFORMAT mod_formats[] = {

	{ MODFILE_SetS3M,
		MODFILE_IsS3M,
		MODFILE_S3MGetFormatID,
		MODFILE_S3MGetDescription,
		MODFILE_S3MGetAuthor,
		MODFILE_S3MGetVersion,
		MODFILE_S3MGetCopyright
	},

	{ MODFILE_SetXM,
		MODFILE_IsXM,
		MODFILE_XMGetFormatID,
		MODFILE_XMGetDescription,
		MODFILE_XMGetAuthor,
		MODFILE_XMGetVersion,
		MODFILE_XMGetCopyright
	},

	{ MODFILE_SetMOD,
		MODFILE_IsMOD,
		MODFILE_MODGetFormatID,
		MODFILE_MODGetDescription,
		MODFILE_MODGetAuthor,
		MODFILE_MODGetVersion,
		MODFILE_MODGetCopyright
	},

	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
/**
 * BOOL MODFILE_Is(u8 *modfile, int modlength);
 *
 * Checks whether the raw data in memory is a valid
 * MOD file of any supported format.
 *
 * Returns TRUE if the data is of any supported
 * format, FALSE if not.
 *
 * Parameters:
 *
 * modfile	 - Pointer to the raw data to be checked
 * modlength - Length of the raw data in bytes
 **/
BOOL MODFILE_Is(u8 *modfile, int modlength) {
	for(int i=0;mod_formats[i].is && mod_formats[i].set;i++)
		if(mod_formats[i].is(modfile, modlength))return TRUE;
	return FALSE;
}
/**
 * int MODFILE_Set(u8 *modfile, int modlength, MODFILE *mod);
 *
 * Processes the raw data of a MOD file of any supported format
 * and copies it to a structure. The structure can then be used
 * as a handle of the MOD file. The original raw data isn't
 * needed by the handle.
 * The function works non-destructive if it fails, ie. it doesn't
 * alter any data in the handle.
 *
 * Returns a value <0 on error.
 *
 * Parameters:
 * modfile	 - A pointer to the raw MOD data
 * modlength - The length of the raw data in bytes
 * mod			 - A pointer to the MOD handle
 **/
int MODFILE_Set(u8 *modfile, int modlength, MODFILE *mod) {
	if((!mod) ||(!modfile) ||(modlength <= 0) || mod->set)return -1;
	for(int i=0,retval;mod_formats[i].set && mod_formats[i].is;i++) {
		if(mod_formats[i].is(modfile, modlength)){
			if((retval = mod_formats[i].set(modfile, modlength, mod)) < 0)return retval;
			mod->set = TRUE;
			return 0;
		}
	}
	return -1;
}
/**
 * int MODFILE_Load(const char *fname, MODFILE *mod);
 *
 * Loads a MOD file of any supported format and copies
 * it to a MODFILE structure. The structure can then
 * be used as a handle of the module.
 *
 * Returns <0 on error.
 *
 * Parameters:
 *
 * fname - Name of the file to be loaded
 * mod	 - Pointer to the MODFILE structure
 **/
int MODFILE_Load(const char *fname, MODFILE *mod) {
	int fd,modlength = 0;
	if(!fname || !mod)return -2;
	if((fd=sceIoOpen(fname, PSP_O_RDONLY ,0777))<0)return -1;
	sceIoLseek(fd, 0, SEEK_END);
	void *file=myMalloc((modlength=sceIoLseek(fd, 0, SEEK_CUR)));
	sceIoLseek(fd, 0, SEEK_SET);
	if(file)sceIoRead(fd,file, modlength);
	sceIoClose(fd);
	int ret = MODFILE_Set(file, modlength, mod);
	myFree(file);
	return ret;
}
/**
 * void MODFILE_Start(MODFILE *mod);
 *
 * Resets all runtime-data in the MODFILE structure,
 * prepares the music for playback and allocates
 * a mixing buffer.
 *
 * Parameters:
 * mod - A pointer too the module handle
 **/
void MODFILE_Start(MODFILE *mod) {
	if(!mod)return;
	mod->speed = mod->start_speed;
	MODFILE_SetBPM(mod, mod->start_tempo);
	mod->pattern_line = 0;
	mod->play_position = 0;
	mod->patterndelay = 0;
	mod->samplescounter = 0;
	mod->speedcounter = 0;
	mod->patternloop_to = 0;
	mod->patternloop_count = 0;
	mod->cur_master_volume = mod->master_volume;
	mod->tempmixbuf = myMalloc(MODFILE_BPM2SamplesPerTick(mod, 32) * sizeof(s32) * 2);
	for(int i = 0; i < mod->nSamples; i++)
		mod->samples[i].middle_c = mod->samples[i].default_middle_c;
	for(int i = 0; i < MODPLAY_MAX_CHANNELS; i++) {
		mod->channels[i].voiceInfo.playing = FALSE;
		mod->channels[i].voiceInfo.volume = 64;
		mod->channels[i].last_instrument = 0;
		for(int c = 0; c < MODPLAY_NUM_COMMANDS; c++) {
			mod->channels[i].effects[c].cur_effect = 255;
			mod->channels[i].effects[c].cur_operand = 255;
			mod->channels[i].effects[c].vibrato_wave = 0;
			mod->channels[i].effects[c].tremolo_wave = 0;
			mod->channels[i].effects[c].tremolo_sintabpos = 0;
			mod->channels[i].effects[c].vibrato_sintabpos = 0;
		}
		mod->channels[i].voiceInfo.panning = mod->channels[i].default_panning;
	}
	mod->playing = TRUE;
}
/**
 * void MODFILE_Stop(MODFILE *mod);
 *
 * Stops music playback and deallocates the
 * mixing buffer.
 **/
void MODFILE_Stop(MODFILE *mod) {
	if(!mod->tempmixbuf)return;
	myFree(mod->tempmixbuf);
	mod->tempmixbuf = NULL;
	mod->playing = FALSE;
}
/**
 * void MODFILE_Player(MODFILE *mod);
 *
 * Calculates mod->mixingbuflen bytes of music data
 * in the format specified with the MODFILE_SetFormat()
 * format and stores the resulting data in the memory
 * pointed to by mod->mixingbuf.
 *
 * Parameters:
 *
 * mod - A pointer to the MODFILE structure which defines
 *			 the music to be calculated.
 **/
void MODFILE_Player(MODFILE *mod) {

	int len = mod->mixingbuflen;
	int remain, l;
	int mixflags;
	u32 retval = 0;
	u8 *buf8 =(u8*)mod->mixingbuf;

	if(mod->mixchannels == 2)
		len >>= 1;
	if(mod->bits == 16)
		len >>= 1;

	mixflags = 0;
	mixflags |= MIXER_USE_S32;
	if(mod->mixchannels == 2)
		mixflags |= MIXER_DEST_STEREO;
	if(mod->bits == 16)
		mixflags |= MIXER_DEST_16BIT;
	if(mod->mixsigned)
		mixflags |= MIXER_DEST_SIGNED;
	remain = len;
	l = 0;

	do {

		int tick_remain = mod->samplespertick - mod->samplescounter;
		int res = MODFILE_Mix(mod, mixflags, &buf8[mix_destbufsize(mixflags) * l], tick_remain <= remain ? tick_remain : remain);
		l += res;
		remain -= res;
		mod->samplescounter += res;
		if(mod->samplescounter >= mod->samplespertick) {
			mod->samplescounter -= mod->samplespertick;
			mod->speedcounter++;
			if(mod->speedcounter >=(mod->speed + mod->patterndelay)) {
				mod->patterndelay = 0;
				retval |= MODFILE_Process(mod);
				mod->speedcounter = 0;
		 	}
//			write(1,retval?"?\n":"!\n",2);
			retval |= MODFILE_EffectHandler(mod);
		}
	} while(remain > 0);
	mod->notebeats = retval;
}
/**
 * void MODFILE_Free(MODFILE *mod);
 *
 * Deallocates all resources occupied by the module
 * in the MODFILE structure after they have been
 * allocated by the MODFILE_Load() or any of the
 * MODFILE_Set*() functions.
 *
 * Parameters:
 *
 * mod - A pointer to the MODFILE structure of which
 *			 the resource shall be deallocated
 **/
void MODFILE_Free(MODFILE *mod) {
	if(!mod || !mod->set)return;
	/* Free patterns */
	if(mod->patterns) {
		for(int i = 0; i < mod->nPatterns; i++) {
			if(mod->patterns[i]) {
				myFree(mod->patterns[i]);
				mod->patterns[i] = NULL;
			}
		}
		myFree(mod->patterns);
		mod->patterns = NULL;
	}
	/* Free instruments */
	if(mod->instruments) {
		for(int i = 0; i < mod->nInstruments; i++) {
			if(mod->instruments[i].envVolume.envPoints) {
				myFree(mod->instruments[i].envVolume.envPoints);
				mod->instruments[i].envVolume.envPoints = NULL;
			}
			if(mod->instruments[i].envPanning.envPoints) {
				myFree(mod->instruments[i].envPanning.envPoints);
				mod->instruments[i].envPanning.envPoints = NULL;
			}
		}
		myFree(mod->instruments);
		mod->instruments = NULL;
	}
	/* Free samples */
	if(mod->samples ) {
		for(int i = 0; i < mod->nSamples; i++) {
			if(mod->samples[i].sampleInfo.sampledata) {
				myFree(mod->samples[i].sampleInfo.sampledata);
				mod->samples[i].sampleInfo.sampledata = NULL;
			}
		}
		myFree(mod->samples);
		mod->samples = NULL;
	}
	if(mod->patternLengths) {
		myFree(mod->patternLengths);
		mod->patternLengths = NULL;
	}
	mod->set = FALSE;
}
/**
 * void MODFILE_Init(MODFILE *mod);
 *
 * Initializes a MODFILE structure for usage. Must
 * be called before the structure can be used by
 * any other function.
 *
 * Parameters:
 *
 * mod - A pointer to the MODFILE structure
 **/
void MODFILE_Init(MODFILE *mod) {
	if(mod)memset(mod, 0, sizeof(MODFILE));
}
/**
 * void MODFILE_SetFormat(MODFILE *mod, int freq, int channels, int bits, BOOL mixsigned);
 *
 * Sets the format of the output audio stream. Must
 * be called prior to calling MODFILE_Start() and
 * MODFILE_Player().
 *
 * Parameters:
 *
 * mod			 - A pointer to the MODFILE structure of
 *						 which the output format shall be changed
 * freq			- Output frequency. Common values are
 *						 11025Hz, 22050Hz and 44100Hz
 * channels	- 1 for mono and 2 for stereo
 * bits			- 8 or 16 are valid values
 * mixsigned - TRUE if the output stream shall consist
 *						 of signed values
 **/
void MODFILE_SetFormat(MODFILE *mod, int freq, int channels, int bits, BOOL mixsigned) {
	mod->playfreq = freq;
	if((channels == 1) ||(channels == 2))
		mod->mixchannels = channels;
	if((bits == 8) ||(bits == 16))
		mod->bits = bits;
	mod->mixsigned = mixsigned;
	if(mod->playing) {
		if(mod->tempmixbuf) {
			myFree(mod->tempmixbuf);
			mod->tempmixbuf = myMalloc(MODFILE_BPM2SamplesPerTick(mod,32) * sizeof(s32) * 2);
		}
		MODFILE_SetBPM(mod, mod->bpm);
	}
}
int MODFILE_AllocSFXChannels(MODFILE *mod, int nChannels) {
	if(!mod || nChannels < 0 || nChannels > 32)return -1;
	if(mod->nChannels + nChannels > MODPLAY_MAX_CHANNELS)return -1;
	mod->nSFXChannels = nChannels;
	for(int i = mod->nChannels; i < mod->nChannels + mod->nSFXChannels; i++)
		mod->channels[i].voiceInfo.enabled = TRUE;
	return 0;
}
MOD_Instrument *MODFILE_MakeInstrument(void *rawData, int nBytes, int nBits) {

	MOD_Instrument *instr;
	MOD_Sample *smpl;
	int shiftVal = nBits == 16 ? 1 : 0;
	int i;

	if(rawData == NULL || nBytes <= 0 ||(nBits != 8 && nBits != 16))
		return NULL;

	instr = myMalloc(sizeof(MOD_Instrument));
	if(instr == NULL)
		return NULL;
	memset(instr, 0, sizeof(MOD_Instrument));

	smpl = myMalloc(sizeof(MOD_Sample));
	if(smpl == NULL) {

		myFree(instr);
		return NULL;
	}
	memset(smpl, 0, sizeof(MOD_Sample));

	for(i = 0; i < 256; i++) {

		instr->samples[i] = smpl;
		instr->note[i] = i;
	}

	instr->name[0] = '\0';
	instr->volumeFade = 4096;

	instr->envVolume.sustain = 255;
	instr->envVolume.loop_start = 255;
	instr->envVolume.loop_end = 255;

	instr->envPanning.sustain = 255;
	instr->envPanning.loop_start = 255;
	instr->envPanning.loop_end = 255;

	smpl->name[0] = '\0';
	smpl->default_volume = 64;
	smpl->middle_c = 8363;
	smpl->default_middle_c = 8363;
	smpl->finetune = 0;
	smpl->relative_note = 0;
	smpl->panning = 32;
	smpl->volume = 64;

	smpl->sampleInfo.length = nBytes >> shiftVal;
	smpl->sampleInfo.loop_start = 0;
	smpl->sampleInfo.loop_end =(nBytes >> shiftVal) - 1;
	smpl->sampleInfo.looped = FALSE;
	smpl->sampleInfo.pingpong = FALSE;
	smpl->sampleInfo.sampledata = rawData;
	smpl->sampleInfo.bit_16 = nBits == 16;
	smpl->sampleInfo.stereo = FALSE;

	return instr;
}
void MODFILE_TriggerSFX(MODFILE *mod, MOD_Instrument *instr, int channel, u8 note) {
	if((!mod) || (!instr))return;
	if(channel < 0 || channel >= mod->nSFXChannels)return;
	if((note & 0x0f) >= 12)return;
	
	int sfxchan = mod->nChannels + channel;
	mod->channels[sfxchan].voiceInfo.playpos = 0;
	mod->channels[sfxchan].voiceInfo.forward = TRUE;
	mod->channels[sfxchan].instrument = instr;
	mod->channels[sfxchan].sample = instr->samples[note];
	mod->channels[sfxchan].voiceInfo.sampleInfo = &mod->channels[sfxchan].sample->sampleInfo;

	u8 dnote =((note >> 4) * 12) +(note & 0x0f);
	dnote += mod->channels[sfxchan].sample->relative_note;
	dnote =((dnote / 12) << 4) |(dnote % 12);
	MODFILE_SetNote(mod, sfxchan,
									mod->channels[sfxchan].instrument->note[dnote],
									mod->channels[sfxchan].sample->middle_c,
									mod->channels[sfxchan].sample->finetune);
	mod->channels[sfxchan].cur_note = dnote;
	mod->channels[sfxchan].voiceInfo.volume = mod->channels[sfxchan].sample->default_volume;
	mod->channels[sfxchan].envVolume.envConfig = &mod->channels[sfxchan].instrument->envVolume;
	EnvTrigger(&mod->channels[sfxchan].envVolume);
	mod->channels[sfxchan].envPanning.envConfig = &mod->channels[sfxchan].instrument->envPanning;
	EnvTrigger(&mod->channels[sfxchan].envPanning);
	mod->channels[sfxchan].volumeFade = 32768;
	mod->channels[sfxchan].volumeFadeDec = 0;
	mod->channels[sfxchan].voiceInfo.playing = TRUE;
	mod->channels[sfxchan].voiceInfo.panning = 128;
}
