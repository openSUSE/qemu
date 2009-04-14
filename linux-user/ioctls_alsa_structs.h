/*
 *  Advanced Linux Sound Architecture
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#ifndef __u8
#define __u8 uint8_t
#define __u16 uint16_t
#define __u32 uint32_t
#define __s8 int8_t
#define __s16 int16_t
#define __s32 int32_t
#endif

#define SNDRV_SB_CSP_MAX_MICROCODE_FILE_SIZE    0x3000
#define HDSP_MATRIX_MIXER_SIZE 2048
#define SNDRV_MASK_MAX  256

typedef struct fm_operator {
	unsigned char am_vib;
	unsigned char ksl_level;
	unsigned char attack_decay;
	unsigned char sustain_release;
	unsigned char wave_select;
} fm_operator_t;

typedef struct {
	unsigned int share_id[4];	/* share id - zero = no sharing */
	unsigned char type;		/* instrument type */

	fm_operator_t op[4];
	unsigned char feedback_connection[2];

	unsigned char echo_delay;
	unsigned char echo_atten;
	unsigned char chorus_spread;
	unsigned char trnsps;
	unsigned char fix_dur;
	unsigned char modes;
	unsigned char fix_key;
} fm_instrument_t;

typedef struct fm_xoperator {
	__u8 am_vib;
	__u8 ksl_level;
	__u8 attack_decay;
	__u8 sustain_release;
	__u8 wave_select;
} fm_xoperator_t;

typedef struct fm_xinstrument {
	__u32 stype;			/* structure type */

	__u32 share_id[4];		/* share id - zero = no sharing */
	__u8 type;			/* instrument type */

	fm_xoperator_t op[4];		/* fm operators */
	__u8 feedback_connection[2];

	__u8 echo_delay;
	__u8 echo_atten;
	__u8 chorus_spread;
	__u8 trnsps;
	__u8 fix_dur;
	__u8 modes;
	__u8 fix_key;
} fm_xinstrument_t;

typedef struct gf1_wave {
	unsigned int share_id[4];	/* share id - zero = no sharing */
	unsigned int format;		/* wave format */

	struct {
		unsigned int number;	/* some other ID for this instrument */
		unsigned int memory;	/* begin of waveform in onboard memory */
		unsigned char *ptr;	/* pointer to waveform in system memory */
	} address;

	unsigned int size;		/* size of waveform in samples */
	unsigned int start;		/* start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned int loop_start;	/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned int loop_end;		/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned short loop_repeat;	/* loop repeat - 0 = forever */

	unsigned char flags;		/* GF1 patch flags */
	unsigned char pad;
	unsigned int sample_rate;	/* sample rate in Hz */
	unsigned int low_frequency;	/* low frequency range */
	unsigned int high_frequency;	/* high frequency range */
	unsigned int root_frequency;	/* root frequency range */
	signed short tune;
	unsigned char balance;
	unsigned char envelope_rate[6];
	unsigned char envelope_offset[6];
	unsigned char tremolo_sweep;
	unsigned char tremolo_rate;
	unsigned char tremolo_depth;
	unsigned char vibrato_sweep;
	unsigned char vibrato_rate;
	unsigned char vibrato_depth;
	unsigned short scale_frequency;
	unsigned short scale_factor;	/* 0-2048 or 0-2 */

	struct gf1_wave *next;
} gf1_wave_t;

typedef struct {
	unsigned short exclusion;
	unsigned short exclusion_group;	/* 0 - none, 1-65535 */

	unsigned char effect1;		/* effect 1 */
	unsigned char effect1_depth;	/* 0-127 */
	unsigned char effect2;		/* effect 2 */
	unsigned char effect2_depth;	/* 0-127 */

	gf1_wave_t *wave;		/* first waveform */
} gf1_instrument_t;

typedef struct gf1_xwave {
	__u32 stype;			/* structure type */

	__u32 share_id[4];		/* share id - zero = no sharing */
	__u32 format;			/* wave format */

	__u32 size;			/* size of waveform in samples */
	__u32 start;			/* start offset in samples * 16 (lowest 4 bits - fraction) */
	__u32 loop_start;		/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	__u32 loop_end;			/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	__u16 loop_repeat;		/* loop repeat - 0 = forever */

	__u8 flags;			/* GF1 patch flags */
	__u8 pad;
	__u32 sample_rate;		/* sample rate in Hz */
	__u32 low_frequency;		/* low frequency range */
	__u32 high_frequency;		/* high frequency range */
	__u32 root_frequency;		/* root frequency range */
	__s16 tune;
	__u8 balance;
	__u8 envelope_rate[6];
	__u8 envelope_offset[6];
	__u8 tremolo_sweep;
	__u8 tremolo_rate;
	__u8 tremolo_depth;
	__u8 vibrato_sweep;
	__u8 vibrato_rate;
	__u8 vibrato_depth;
	__u16 scale_frequency;
	__u16 scale_factor;		/* 0-2048 or 0-2 */
} gf1_xwave_t;

typedef struct gf1_xinstrument {
	__u32 stype;

	__u16 exclusion;
	__u16 exclusion_group;		/* 0 - none, 1-65535 */

	__u8 effect1;			/* effect 1 */
	__u8 effect1_depth;		/* 0-127 */
	__u8 effect2;			/* effect 2 */
	__u8 effect2_depth;		/* 0-127 */
} gf1_xinstrument_t;

typedef struct gf1_info {
	unsigned char flags;		/* supported wave flags */
	unsigned char pad[3];
	unsigned int features;		/* supported features */
	unsigned int max8_len;		/* maximum 8-bit wave length */
	unsigned int max16_len;		/* maximum 16-bit wave length */
} gf1_info_t;

typedef struct iwffff_wave {
	unsigned int share_id[4];	/* share id - zero = no sharing */
	unsigned int format;		/* wave format */

	struct {
		unsigned int number;	/* some other ID for this wave */
		unsigned int memory;	/* begin of waveform in onboard memory */
		unsigned char *ptr;	/* pointer to waveform in system memory */
	} address;

	unsigned int size;		/* size of waveform in samples */
	unsigned int start;		/* start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned int loop_start;	/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned int loop_end;		/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned short loop_repeat;	/* loop repeat - 0 = forever */
	unsigned int sample_ratio;	/* sample ratio (44100 * 1024 / rate) */
	unsigned char attenuation;	/* 0 - 127 (no corresponding midi controller) */
	unsigned char low_note;		/* lower frequency range for this waveform */
	unsigned char high_note;	/* higher frequency range for this waveform */
	unsigned char pad;

	struct iwffff_wave *next;
} iwffff_wave_t;

typedef struct iwffff_lfo {
	unsigned short freq;		/* (0-2047) 0.01Hz - 21.5Hz */
	signed short depth;		/* volume +- (0-255) 0.48675dB/step */
	signed short sweep;		/* 0 - 950 deciseconds */
	unsigned char shape;		/* see to IWFFFF_LFO_SHAPE_XXXX */
	unsigned char delay;		/* 0 - 255 deciseconds */
} iwffff_lfo_t;

typedef struct iwffff_env_point {
	unsigned short offset;
	unsigned short rate;
} iwffff_env_point_t;

typedef struct iwffff_env_record {
	unsigned short nattack;
	unsigned short nrelease;
	unsigned short sustain_offset;
	unsigned short sustain_rate;
	unsigned short release_rate;
	unsigned char hirange;
	unsigned char pad;
	struct iwffff_env_record *next;
	/* points are stored here */
	/* count of points = nattack + nrelease */
} iwffff_env_record_t;

typedef struct iwffff_env {
	unsigned char flags;
	unsigned char mode;
	unsigned char index;
	unsigned char pad;
	struct iwffff_env_record *record;
} iwffff_env_t;

typedef struct iwffff_layer {
	unsigned char flags;
	unsigned char velocity_mode;
	unsigned char layer_event;
	unsigned char low_range;	/* range for layer based */
	unsigned char high_range;	/* on either velocity or frequency */
	unsigned char pan;		/* pan offset from CC1 (0 left - 127 right) */
	unsigned char pan_freq_scale;	/* position based on frequency (0-127) */
	unsigned char attenuation;	/* 0-127 (no corresponding midi controller) */
	iwffff_lfo_t tremolo;		/* tremolo effect */
	iwffff_lfo_t vibrato;		/* vibrato effect */
	unsigned short freq_scale;	/* 0-2048, 1024 is equal to semitone scaling */
	unsigned char freq_center;	/* center for keyboard frequency scaling */
	unsigned char pad;
	iwffff_env_t penv;		/* pitch envelope */
	iwffff_env_t venv;		/* volume envelope */

	iwffff_wave_t *wave;
	struct iwffff_layer *next;
} iwffff_layer_t;

typedef struct {
	unsigned short exclusion;
	unsigned short layer_type;
	unsigned short exclusion_group;	/* 0 - none, 1-65535 */

	unsigned char effect1;		/* effect 1 */
	unsigned char effect1_depth;	/* 0-127 */
	unsigned char effect2;		/* effect 2 */
	unsigned char effect2_depth;	/* 0-127 */

	iwffff_layer_t *layer;		/* first layer */
} iwffff_instrument_t;

typedef struct iwffff_xwave {
	__u32 stype;			/* structure type */

	__u32 share_id[4];		/* share id - zero = no sharing */

	__u32 format;			/* wave format */
	__u32 offset;			/* offset to ROM (address) */

	__u32 size;			/* size of waveform in samples */
	__u32 start;			/* start offset in samples * 16 (lowest 4 bits - fraction) */
	__u32 loop_start;		/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	__u32 loop_end;			/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	__u16 loop_repeat;		/* loop repeat - 0 = forever */
	__u32 sample_ratio;		/* sample ratio (44100 * 1024 / rate) */
	__u8 attenuation;		/* 0 - 127 (no corresponding midi controller) */
	__u8 low_note;			/* lower frequency range for this waveform */
	__u8 high_note;			/* higher frequency range for this waveform */
	__u8 pad;
} iwffff_xwave_t;

typedef struct iwffff_xlfo {
	__u16 freq;			/* (0-2047) 0.01Hz - 21.5Hz */
	__s16 depth;			/* volume +- (0-255) 0.48675dB/step */
	__s16 sweep;			/* 0 - 950 deciseconds */
	__u8 shape;			/* see to ULTRA_IW_LFO_SHAPE_XXXX */
	__u8 delay;			/* 0 - 255 deciseconds */
} iwffff_xlfo_t;

typedef struct iwffff_xenv_point {
	__u16 offset;
	__u16 rate;
} iwffff_xenv_point_t;

typedef struct iwffff_xenv_record {
	__u32 stype;
	__u16 nattack;
	__u16 nrelease;
	__u16 sustain_offset;
	__u16 sustain_rate;
	__u16 release_rate;
	__u8 hirange;
	__u8 pad;
	/* points are stored here.. */
	/* count of points = nattack + nrelease */
} iwffff_xenv_record_t;

typedef struct iwffff_xenv {
	__u8 flags;
	__u8 mode;
	__u8 index;
	__u8 pad;
} iwffff_xenv_t;

typedef struct iwffff_xlayer {
	__u32 stype;
	__u8 flags;
	__u8 velocity_mode;
	__u8 layer_event;
	__u8 low_range;			/* range for layer based */
	__u8 high_range;		/* on either velocity or frequency */
	__u8 pan;			/* pan offset from CC1 (0 left - 127 right) */
	__u8 pan_freq_scale;		/* position based on frequency (0-127) */
	__u8 attenuation;		/* 0-127 (no corresponding midi controller) */
	iwffff_xlfo_t tremolo;		/* tremolo effect */
	iwffff_xlfo_t vibrato;		/* vibrato effect */
	__u16 freq_scale;		/* 0-2048, 1024 is equal to semitone scaling */
	__u8 freq_center;		/* center for keyboard frequency scaling */
	__u8 pad;
	iwffff_xenv_t penv;		/* pitch envelope */
	iwffff_xenv_t venv;		/* volume envelope */
} iwffff_xlayer_t;

typedef struct iwffff_xinstrument {
	__u32 stype;

	__u16 exclusion;
	__u16 layer_type;
	__u16 exclusion_group;		/* 0 - none, 1-65535 */

	__u8 effect1;			/* effect 1 */
	__u8 effect1_depth;		/* 0-127 */
	__u8 effect2;			/* effect 2 */
	__u8 effect2_depth;		/* 0-127 */
} iwffff_xinstrument_t;

typedef struct {
	__u8 iwave[8];
	__u8 revision;
	__u8 series_number;
	__u8 series_name[16];
	__u8 date[10];
	__u16 vendor_revision_major;
	__u16 vendor_revision_minor;
	__u32 rom_size;
	__u8 copyright[128];
	__u8 vendor_name[64];
	__u8 description[128];
} iwffff_rom_header_t;

typedef struct iwffff_info {
	unsigned int format;		/* supported format bits */
	unsigned int effects;		/* supported effects (1 << IWFFFF_EFFECT*) */
	unsigned int lfos;		/* LFO effects */
	unsigned int max8_len;		/* maximum 8-bit wave length */
	unsigned int max16_len;		/* maximum 16-bit wave length */
} iwffff_info_t;

typedef struct simple_instrument_info {
	unsigned int format;		/* supported format bits */
	unsigned int effects;		/* supported effects (1 << SIMPLE_EFFECT_*) */
	unsigned int max8_len;		/* maximum 8-bit wave length */
	unsigned int max16_len;		/* maximum 16-bit wave length */
} simple_instrument_info_t;

typedef struct {
	unsigned int share_id[4];	/* share id - zero = no sharing */
	unsigned int format;		/* wave format */

	struct {
		unsigned int number;	/* some other ID for this instrument */
		unsigned int memory;	/* begin of waveform in onboard memory */
		unsigned char *ptr;	/* pointer to waveform in system memory */
	} address;

	unsigned int size;		/* size of waveform in samples */
	unsigned int start;		/* start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned int loop_start;	/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned int loop_end;		/* loop end offset in samples * 16 (lowest 4 bits - fraction) */
	unsigned short loop_repeat;	/* loop repeat - 0 = forever */

	unsigned char effect1;		/* effect 1 */
	unsigned char effect1_depth;	/* 0-127 */
	unsigned char effect2;		/* effect 2 */
	unsigned char effect2_depth;	/* 0-127 */
} simple_instrument_t;

typedef struct simple_xinstrument {
	__u32 stype;

	__u32 share_id[4];		/* share id - zero = no sharing */
	__u32 format;			/* wave format */

	__u32 size;			/* size of waveform in samples */
	__u32 start;			/* start offset in samples * 16 (lowest 4 bits - fraction) */
	__u32 loop_start;		/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	__u32 loop_end;			/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	__u16 loop_repeat;		/* loop repeat - 0 = forever */

	__u8 effect1;			/* effect 1 */
	__u8 effect1_depth;		/* 0-127 */
	__u8 effect2;			/* effect 2 */
	__u8 effect2_depth;		/* 0-127 */
} simple_xinstrument_t;

typedef unsigned char sndrv_seq_event_type_t;

/** event address */
struct sndrv_seq_addr {
	unsigned char client;	/**< Client number:         0..255, 255 = broadcast to all clients */
	unsigned char port;	/**< Port within client:    0..255, 255 = broadcast to all ports */
};

/** port connection */
struct sndrv_seq_connect {
	struct sndrv_seq_addr sender;
	struct sndrv_seq_addr dest;
};

struct sndrv_seq_ev_note {
	unsigned char channel;
	unsigned char note;
	unsigned char velocity;
	unsigned char off_velocity;	/* only for SNDRV_SEQ_EVENT_NOTE */
	unsigned int duration;		/* only for SNDRV_SEQ_EVENT_NOTE */
};

	/* controller event */
struct sndrv_seq_ev_ctrl {
	unsigned char channel;
	unsigned char unused1, unused2, unused3;	/* pad */
	unsigned int param;
	signed int value;
};

	/* generic set of bytes (12x8 bit) */
struct sndrv_seq_ev_raw8 {
	unsigned char d[12];	/* 8 bit value */
};

	/* generic set of integers (3x32 bit) */
struct sndrv_seq_ev_raw32 {
	unsigned int d[3];	/* 32 bit value */
};

	/* external stored data */
struct sndrv_seq_ev_ext {
	unsigned int len;	/* length of data */
	void *ptr;		/* pointer to data (note: maybe 64-bit) */
} __attribute__((packed));

/* Instrument cluster type */
typedef unsigned int sndrv_seq_instr_cluster_t;

/* Instrument type */
struct sndrv_seq_instr {
	sndrv_seq_instr_cluster_t cluster;
	unsigned int std;		/* the upper byte means a private instrument (owner - client #) */
	unsigned short bank;
	unsigned short prg;
};

	/* sample number */
struct sndrv_seq_ev_sample {
	unsigned int std;
	unsigned short bank;
	unsigned short prg;
};

	/* sample cluster */
struct sndrv_seq_ev_cluster {
	sndrv_seq_instr_cluster_t cluster;
};

	/* sample position */
typedef unsigned int sndrv_seq_position_t; /* playback position (in samples) * 16 */

	/* sample stop mode */
enum sndrv_seq_stop_mode {
	SAMPLE_STOP_IMMEDIATELY = 0,	/* terminate playing immediately */
	SAMPLE_STOP_VENVELOPE = 1,	/* finish volume envelope */
	SAMPLE_STOP_LOOP = 2		/* terminate loop and finish wave */
};

	/* sample frequency */
typedef int sndrv_seq_frequency_t; /* playback frequency in HZ * 16 */

	/* sample volume control; if any value is set to -1 == do not change */
struct sndrv_seq_ev_volume {
	signed short volume;	/* range: 0-16383 */
	signed short lr;	/* left-right balance; range: 0-16383 */
	signed short fr;	/* front-rear balance; range: 0-16383 */
	signed short du;	/* down-up balance; range: 0-16383 */
};

	/* simple loop redefinition */
struct sndrv_seq_ev_loop {
	unsigned int start;	/* loop start (in samples) * 16 */
	unsigned int end;	/* loop end (in samples) * 16 */
};

struct sndrv_seq_ev_sample_control {
	unsigned char channel;
	unsigned char unused1, unused2, unused3;	/* pad */
	union {
		struct sndrv_seq_ev_sample sample;
		struct sndrv_seq_ev_cluster cluster;
		sndrv_seq_position_t position;
		int stop_mode;
		sndrv_seq_frequency_t frequency;
		struct sndrv_seq_ev_volume volume;
		struct sndrv_seq_ev_loop loop;
		unsigned char raw8[8];
	} param;
};



/* INSTR_BEGIN event */
struct sndrv_seq_ev_instr_begin {
	int timeout;		/* zero = forever, otherwise timeout in ms */
};

struct sndrv_seq_result {
	int event;		/* processed event type */
	int result;
};


struct sndrv_seq_real_time {
	unsigned int tv_sec;	/* seconds */
	unsigned int tv_nsec;	/* nanoseconds */
};

typedef unsigned int sndrv_seq_tick_time_t;	/* midi ticks */

union sndrv_seq_timestamp {
	sndrv_seq_tick_time_t tick;
	struct sndrv_seq_real_time time;
};

struct sndrv_seq_queue_skew {
	unsigned int value;
	unsigned int base;
};

	/* queue timer control */
struct sndrv_seq_ev_queue_control {
	unsigned char queue;			/* affected queue */
	unsigned char pad[3];			/* reserved */
	union {
		signed int value;		/* affected value (e.g. tempo) */
		union sndrv_seq_timestamp time;	/* time */
		unsigned int position;		/* sync position */
		struct sndrv_seq_queue_skew skew;
		unsigned int d32[2];
		unsigned char d8[8];
	} param;
};

	/* quoted event - inside the kernel only */
struct sndrv_seq_ev_quote {
	struct sndrv_seq_addr origin;		/* original sender */
	unsigned short value;		/* optional data */
	struct sndrv_seq_event *event;		/* quoted event */
} __attribute__((packed));


	/* sequencer event */
struct sndrv_seq_event {
	sndrv_seq_event_type_t type;	/* event type */
	unsigned char flags;		/* event flags */
	char tag;

	unsigned char queue;		/* schedule queue */
	union sndrv_seq_timestamp time;	/* schedule time */


	struct sndrv_seq_addr source;	/* source address */
	struct sndrv_seq_addr dest;	/* destination address */

	union {				/* event data... */
		struct sndrv_seq_ev_note note;
		struct sndrv_seq_ev_ctrl control;
		struct sndrv_seq_ev_raw8 raw8;
		struct sndrv_seq_ev_raw32 raw32;
		struct sndrv_seq_ev_ext ext;
		struct sndrv_seq_ev_queue_control queue;
		union sndrv_seq_timestamp time;
		struct sndrv_seq_addr addr;
		struct sndrv_seq_connect connect;
		struct sndrv_seq_result result;
		struct sndrv_seq_ev_instr_begin instr_begin;
		struct sndrv_seq_ev_sample_control sample;
		struct sndrv_seq_ev_quote quote;
	} data;
};


/*
 * bounce event - stored as variable size data
 */
struct sndrv_seq_event_bounce {
	int err;
	struct sndrv_seq_event event;
	/* external data follows here. */
};

struct sndrv_seq_system_info {
	int queues;			/* maximum queues count */
	int clients;			/* maximum clients count */
	int ports;			/* maximum ports per client */
	int channels;			/* maximum channels per port */
	int cur_clients;		/* current clients */
	int cur_queues;			/* current queues */
	char reserved[24];
};

struct sndrv_seq_running_info {
	unsigned char client;		/* client id */
	unsigned char big_endian;	/* 1 = big-endian */
	unsigned char cpu_mode;		/* 4 = 32bit, 8 = 64bit */
	unsigned char pad;		/* reserved */
	unsigned char reserved[12];
};

enum sndrv_seq_client_type {
	NO_CLIENT       = 0,
	USER_CLIENT     = 1,
	KERNEL_CLIENT   = 2
};

struct sndrv_seq_client_info {
	int client;			/* client number to inquire */
	int type;			/* client type */
	char name[64];			/* client name */
	unsigned int filter;		/* filter flags */
	unsigned char multicast_filter[8]; /* multicast filter bitmap */
	unsigned char event_filter[32];	/* event filter bitmap */
	int num_ports;			/* RO: number of ports */
	int event_lost;			/* number of lost events */
	char reserved[64];		/* for future use */
};

struct sndrv_seq_client_pool {
	int client;			/* client number to inquire */
	int output_pool;		/* outgoing (write) pool size */
	int input_pool;			/* incoming (read) pool size */
	int output_room;		/* minimum free pool size for select/blocking mode */
	int output_free;		/* unused size */
	int input_free;			/* unused size */
	char reserved[64];
};

struct sndrv_seq_remove_events {
	unsigned int  remove_mode;	/* Flags that determine what gets removed */

	union sndrv_seq_timestamp time;

	unsigned char queue;	/* Queue for REMOVE_DEST */
	struct sndrv_seq_addr dest;	/* Address for REMOVE_DEST */
	unsigned char channel;	/* Channel for REMOVE_DEST */

	int  type;	/* For REMOVE_EVENT_TYPE */
	char  tag;	/* Tag for REMOVE_TAG */

	int  reserved[10];	/* To allow for future binary compatibility */

};

struct sndrv_seq_port_info {
	struct sndrv_seq_addr addr;	/* client/port numbers */
	char name[64];			/* port name */

	unsigned int capability;	/* port capability bits */
	unsigned int type;		/* port type bits */
	int midi_channels;		/* channels per MIDI port */
	int midi_voices;		/* voices per MIDI port */
	int synth_voices;		/* voices per SYNTH port */

	int read_use;			/* R/O: subscribers for output (from this port) */
	int write_use;			/* R/O: subscribers for input (to this port) */

	void *kernel;			/* reserved for kernel use (must be NULL) */
	unsigned int flags;		/* misc. conditioning */
	unsigned char time_queue;	/* queue # for timestamping */
	char reserved[59];		/* for future use */
};

struct sndrv_seq_queue_info {
	int queue;		/* queue id */

	/*
	 *  security settings, only owner of this queue can start/stop timer
	 *  etc. if the queue is locked for other clients
	 */
	int owner;		/* client id for owner of the queue */
	int locked:1;		/* timing queue locked for other queues */
	char name[64];		/* name of this queue */
	unsigned int flags;	/* flags */
	char reserved[60];	/* for future use */

};

struct sndrv_seq_queue_status {
	int queue;			/* queue id */
	int events;			/* read-only - queue size */
	sndrv_seq_tick_time_t tick;	/* current tick */
	struct sndrv_seq_real_time time; /* current time */
	int running;			/* running state of queue */
	int flags;			/* various flags */
	char reserved[64];		/* for the future */
};

struct sndrv_seq_queue_tempo {
	int queue;			/* sequencer queue */
	unsigned int tempo;		/* current tempo, us/tick */
	int ppq;			/* time resolution, ticks/quarter */
	unsigned int skew_value;	/* queue skew */
	unsigned int skew_base;		/* queue skew base */
	char reserved[24];		/* for the future */
};

struct sndrv_timer_id {
	int dev_class;
	int dev_sclass;
	int card;
	int device;
	int subdevice;
};

struct sndrv_seq_queue_timer {
	int queue;			/* sequencer queue */
	int type;			/* source timer type */
	union {
		struct {
			struct sndrv_timer_id id;	/* ALSA's timer ID */
			unsigned int resolution;	/* resolution in Hz */
		} alsa;
	} u;
	char reserved[64];		/* for the future use */
};

struct sndrv_seq_queue_client {
	int queue;		/* sequencer queue */
	int client;		/* sequencer client */
	int used;		/* queue is used with this client
				   (must be set for accepting events) */
	/* per client watermarks */
	char reserved[64];	/* for future use */
};

struct sndrv_seq_port_subscribe {
	struct sndrv_seq_addr sender;	/* sender address */
	struct sndrv_seq_addr dest;	/* destination address */
	unsigned int voices;		/* number of voices to be allocated (0 = don't care) */
	unsigned int flags;		/* modes */
	unsigned char queue;		/* input time-stamp queue (optional) */
	unsigned char pad[3];		/* reserved */
	char reserved[64];
};

struct sndrv_seq_query_subs {
	struct sndrv_seq_addr root;	/* client/port id to be searched */
	int type;		/* READ or WRITE */
	int index;		/* 0..N-1 */
	int num_subs;		/* R/O: number of subscriptions on this port */
	struct sndrv_seq_addr addr;	/* R/O: result */
	unsigned char queue;	/* R/O: result */
	unsigned int flags;	/* R/O: result */
	char reserved[64];	/* for future use */
};

/* size of ROM/RAM */
typedef unsigned int sndrv_seq_instr_size_t;

struct sndrv_seq_instr_info {
	int result;			/* operation result */
	unsigned int formats[8];	/* bitmap of supported formats */
	int ram_count;			/* count of RAM banks */
	sndrv_seq_instr_size_t ram_sizes[16]; /* size of RAM banks */
	int rom_count;			/* count of ROM banks */
	sndrv_seq_instr_size_t rom_sizes[8]; /* size of ROM banks */
	char reserved[128];
};

struct sndrv_seq_instr_status {
	int result;			/* operation result */
	sndrv_seq_instr_size_t free_ram[16]; /* free RAM in banks */
	int instrument_count;		/* count of downloaded instruments */
	char reserved[128];
};

struct sndrv_seq_instr_format_info {
	char format[16];		/* format identifier - SNDRV_SEQ_INSTR_ID_* */
	unsigned int len;		/* max data length (without this structure) */
};

struct sndrv_seq_instr_format_info_result {
	int result;			/* operation result */
	char format[16];		/* format identifier */
	unsigned int len;		/* filled data length (without this structure) */
};

struct sndrv_seq_instr_data {
	char name[32];			/* instrument name */
	char reserved[16];		/* for the future use */
	int type;			/* instrument type */
	union {
		char format[16];	/* format identifier */
		struct sndrv_seq_instr alias;
	} data;
};

struct sndrv_seq_instr_header {
	union {
		struct sndrv_seq_instr instr;
		sndrv_seq_instr_cluster_t cluster;
	} id;				/* instrument identifier */
	unsigned int cmd;		/* get/put/free command */
	unsigned int flags;		/* query flags (only for get) */
	unsigned int len;		/* real instrument data length (without header) */
	int result;			/* operation result */
	char reserved[16];		/* for the future */
	struct sndrv_seq_instr_data data; /* instrument data (for put/get result) */
};

struct sndrv_seq_instr_cluster_set {
	sndrv_seq_instr_cluster_t cluster; /* cluster identifier */
	char name[32];			/* cluster name */
	int priority;			/* cluster priority */
	char reserved[64];		/* for the future use */
};

struct sndrv_seq_instr_cluster_get {
	sndrv_seq_instr_cluster_t cluster; /* cluster identifier */
	char name[32];			/* cluster name */
	int priority;			/* cluster priority */
	char reserved[64];		/* for the future use */
};

typedef struct snd_dm_fm_info {
	unsigned char fm_mode;		/* OPL mode, see SNDRV_DM_FM_MODE_XXX */
	unsigned char rhythm;		/* percussion mode flag */
} snd_dm_fm_info_t;

typedef struct snd_dm_fm_voice {
	unsigned char op;		/* operator cell (0 or 1) */
	unsigned char voice;		/* FM voice (0 to 17) */

	unsigned char am;		/* amplitude modulation */
	unsigned char vibrato;		/* vibrato effect */
	unsigned char do_sustain;	/* sustain phase */
	unsigned char kbd_scale;	/* keyboard scaling */
	unsigned char harmonic;		/* 4 bits: harmonic and multiplier */
	unsigned char scale_level;	/* 2 bits: decrease output freq rises */
	unsigned char volume;		/* 6 bits: volume */

	unsigned char attack;		/* 4 bits: attack rate */
	unsigned char decay;		/* 4 bits: decay rate */
	unsigned char sustain;		/* 4 bits: sustain level */
	unsigned char release;		/* 4 bits: release rate */

	unsigned char feedback;		/* 3 bits: feedback for op0 */
	unsigned char connection;	/* 0 for serial, 1 for parallel */
	unsigned char left;		/* stereo left */
	unsigned char right;		/* stereo right */
	unsigned char waveform;		/* 3 bits: waveform shape */
} snd_dm_fm_voice_t;

typedef struct snd_dm_fm_note {
	unsigned char voice;	/* 0-17 voice channel */
	unsigned char octave;	/* 3 bits: what octave to play */
	unsigned int fnum;	/* 10 bits: frequency number */
	unsigned char key_on;	/* set for active, clear for silent */
} snd_dm_fm_note_t;

typedef struct snd_dm_fm_params {
	unsigned char am_depth;		/* amplitude modulation depth (1=hi) */
	unsigned char vib_depth;	/* vibrato depth (1=hi) */
	unsigned char kbd_split;	/* keyboard split */
	unsigned char rhythm;		/* percussion mode select */

	/* This block is the percussion instrument data */
	unsigned char bass;
	unsigned char snare;
	unsigned char tomtom;
	unsigned char cymbal;
	unsigned char hihat;
} snd_dm_fm_params_t;

#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SNDRV_LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#define SNDRV_BIG_ENDIAN
#else
#error "Unsupported endian..."
#endif

#include <sys/time.h>
#include <sys/types.h>

struct sndrv_aes_iec958 {
	unsigned char status[24];	/* AES/IEC958 channel status bits */
	unsigned char subcode[147];	/* AES/IEC958 subcode bits */
	unsigned char pad;		/* nothing */
	unsigned char dig_subframe[4];	/* AES/IEC958 subframe bits */
};

enum sndrv_hwdep_iface {
	SNDRV_HWDEP_IFACE_OPL2 = 0,
	SNDRV_HWDEP_IFACE_OPL3,
	SNDRV_HWDEP_IFACE_OPL4,
	SNDRV_HWDEP_IFACE_SB16CSP,	/* Creative Signal Processor */
	SNDRV_HWDEP_IFACE_EMU10K1,	/* FX8010 processor in EMU10K1 chip */
	SNDRV_HWDEP_IFACE_YSS225,	/* Yamaha FX processor */
	SNDRV_HWDEP_IFACE_ICS2115,	/* Wavetable synth */
	SNDRV_HWDEP_IFACE_SSCAPE,	/* Ensoniq SoundScape ISA card (MC68EC000) */
	SNDRV_HWDEP_IFACE_VX,		/* Digigram VX cards */
	SNDRV_HWDEP_IFACE_MIXART,	/* Digigram miXart cards */
	SNDRV_HWDEP_IFACE_USX2Y,	/* Tascam US122, US224 & US428 usb */
	SNDRV_HWDEP_IFACE_EMUX_WAVETABLE, /* EmuX wavetable */
	SNDRV_HWDEP_IFACE_BLUETOOTH,	/* Bluetooth audio */
	SNDRV_HWDEP_IFACE_USX2Y_PCM,	/* Tascam US122, US224 & US428 rawusb pcm */
	SNDRV_HWDEP_IFACE_PCXHR,	/* Digigram PCXHR */
	SNDRV_HWDEP_IFACE_SB_RC,	/* SB Extigy/Audigy2NX remote control */

	/* Don't forget to change the following: */
	SNDRV_HWDEP_IFACE_LAST = SNDRV_HWDEP_IFACE_SB_RC
};

struct sndrv_hwdep_info {
	unsigned int device;		/* WR: device number */
	int card;			/* R: card number */
	unsigned char id[64];		/* ID (user selectable) */
	unsigned char name[80];		/* hwdep name */
	int iface;			/* hwdep interface */
	unsigned char reserved[64];	/* reserved for future */
};

/* generic DSP loader */
struct sndrv_hwdep_dsp_status {
	unsigned int version;		/* R: driver-specific version */
	unsigned char id[32];		/* R: driver-specific ID string */
	unsigned int num_dsps;		/* R: number of DSP images to transfer */
	unsigned int dsp_loaded;	/* R: bit flags indicating the loaded DSPs */
	unsigned int chip_ready;	/* R: 1 = initialization finished */
	unsigned char reserved[16];	/* reserved for future use */
};

struct sndrv_hwdep_dsp_image {
	unsigned int index;		/* W: DSP index */
	unsigned char name[64];		/* W: ID (e.g. file name) */
	unsigned char *image;		/* W: binary image */
	size_t length;			/* W: size of image in bytes */
	unsigned long driver_data;	/* W: driver-specific data */
};

typedef unsigned long sndrv_pcm_uframes_t;
typedef long sndrv_pcm_sframes_t;

enum sndrv_pcm_class {
	SNDRV_PCM_CLASS_GENERIC = 0,	/* standard mono or stereo device */
	SNDRV_PCM_CLASS_MULTI,		/* multichannel device */
	SNDRV_PCM_CLASS_MODEM,		/* software modem class */
	SNDRV_PCM_CLASS_DIGITIZER,	/* digitizer class */
	/* Don't forget to change the following: */
	SNDRV_PCM_CLASS_LAST = SNDRV_PCM_CLASS_DIGITIZER,
};

enum sndrv_pcm_subclass {
	SNDRV_PCM_SUBCLASS_GENERIC_MIX = 0, /* mono or stereo subdevices are mixed together */
	SNDRV_PCM_SUBCLASS_MULTI_MIX,	/* multichannel subdevices are mixed together */
	/* Don't forget to change the following: */
	SNDRV_PCM_SUBCLASS_LAST = SNDRV_PCM_SUBCLASS_MULTI_MIX,
};

enum sndrv_pcm_stream {
	SNDRV_PCM_STREAM_PLAYBACK = 0,
	SNDRV_PCM_STREAM_CAPTURE,
	SNDRV_PCM_STREAM_LAST = SNDRV_PCM_STREAM_CAPTURE,
};

enum sndrv_pcm_access {
	SNDRV_PCM_ACCESS_MMAP_INTERLEAVED = 0,	/* interleaved mmap */
	SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED, 	/* noninterleaved mmap */
	SNDRV_PCM_ACCESS_MMAP_COMPLEX,		/* complex mmap */
	SNDRV_PCM_ACCESS_RW_INTERLEAVED,	/* readi/writei */
	SNDRV_PCM_ACCESS_RW_NONINTERLEAVED,	/* readn/writen */
	SNDRV_PCM_ACCESS_LAST = SNDRV_PCM_ACCESS_RW_NONINTERLEAVED,
};

enum sndrv_pcm_format {
	SNDRV_PCM_FORMAT_S8 = 0,
	SNDRV_PCM_FORMAT_U8,
	SNDRV_PCM_FORMAT_S16_LE,
	SNDRV_PCM_FORMAT_S16_BE,
	SNDRV_PCM_FORMAT_U16_LE,
	SNDRV_PCM_FORMAT_U16_BE,
	SNDRV_PCM_FORMAT_S24_LE,	/* low three bytes */
	SNDRV_PCM_FORMAT_S24_BE,	/* low three bytes */
	SNDRV_PCM_FORMAT_U24_LE,	/* low three bytes */
	SNDRV_PCM_FORMAT_U24_BE,	/* low three bytes */
	SNDRV_PCM_FORMAT_S32_LE,
	SNDRV_PCM_FORMAT_S32_BE,
	SNDRV_PCM_FORMAT_U32_LE,
	SNDRV_PCM_FORMAT_U32_BE,
	SNDRV_PCM_FORMAT_FLOAT_LE,	/* 4-byte float, IEEE-754 32-bit, range -1.0 to 1.0 */
	SNDRV_PCM_FORMAT_FLOAT_BE,	/* 4-byte float, IEEE-754 32-bit, range -1.0 to 1.0 */
	SNDRV_PCM_FORMAT_FLOAT64_LE,	/* 8-byte float, IEEE-754 64-bit, range -1.0 to 1.0 */
	SNDRV_PCM_FORMAT_FLOAT64_BE,	/* 8-byte float, IEEE-754 64-bit, range -1.0 to 1.0 */
	SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE,	/* IEC-958 subframe, Little Endian */
	SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE,	/* IEC-958 subframe, Big Endian */
	SNDRV_PCM_FORMAT_MU_LAW,
	SNDRV_PCM_FORMAT_A_LAW,
	SNDRV_PCM_FORMAT_IMA_ADPCM,
	SNDRV_PCM_FORMAT_MPEG,
	SNDRV_PCM_FORMAT_GSM,
	SNDRV_PCM_FORMAT_SPECIAL = 31,
	SNDRV_PCM_FORMAT_S24_3LE = 32,	/* in three bytes */
	SNDRV_PCM_FORMAT_S24_3BE,	/* in three bytes */
	SNDRV_PCM_FORMAT_U24_3LE,	/* in three bytes */
	SNDRV_PCM_FORMAT_U24_3BE,	/* in three bytes */
	SNDRV_PCM_FORMAT_S20_3LE,	/* in three bytes */
	SNDRV_PCM_FORMAT_S20_3BE,	/* in three bytes */
	SNDRV_PCM_FORMAT_U20_3LE,	/* in three bytes */
	SNDRV_PCM_FORMAT_U20_3BE,	/* in three bytes */
	SNDRV_PCM_FORMAT_S18_3LE,	/* in three bytes */
	SNDRV_PCM_FORMAT_S18_3BE,	/* in three bytes */
	SNDRV_PCM_FORMAT_U18_3LE,	/* in three bytes */
	SNDRV_PCM_FORMAT_U18_3BE,	/* in three bytes */
	SNDRV_PCM_FORMAT_LAST = SNDRV_PCM_FORMAT_U18_3BE,

#ifdef SNDRV_LITTLE_ENDIAN
	SNDRV_PCM_FORMAT_S16 = SNDRV_PCM_FORMAT_S16_LE,
	SNDRV_PCM_FORMAT_U16 = SNDRV_PCM_FORMAT_U16_LE,
	SNDRV_PCM_FORMAT_S24 = SNDRV_PCM_FORMAT_S24_LE,
	SNDRV_PCM_FORMAT_U24 = SNDRV_PCM_FORMAT_U24_LE,
	SNDRV_PCM_FORMAT_S32 = SNDRV_PCM_FORMAT_S32_LE,
	SNDRV_PCM_FORMAT_U32 = SNDRV_PCM_FORMAT_U32_LE,
	SNDRV_PCM_FORMAT_FLOAT = SNDRV_PCM_FORMAT_FLOAT_LE,
	SNDRV_PCM_FORMAT_FLOAT64 = SNDRV_PCM_FORMAT_FLOAT64_LE,
	SNDRV_PCM_FORMAT_IEC958_SUBFRAME = SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE,
#endif
#ifdef SNDRV_BIG_ENDIAN
	SNDRV_PCM_FORMAT_S16 = SNDRV_PCM_FORMAT_S16_BE,
	SNDRV_PCM_FORMAT_U16 = SNDRV_PCM_FORMAT_U16_BE,
	SNDRV_PCM_FORMAT_S24 = SNDRV_PCM_FORMAT_S24_BE,
	SNDRV_PCM_FORMAT_U24 = SNDRV_PCM_FORMAT_U24_BE,
	SNDRV_PCM_FORMAT_S32 = SNDRV_PCM_FORMAT_S32_BE,
	SNDRV_PCM_FORMAT_U32 = SNDRV_PCM_FORMAT_U32_BE,
	SNDRV_PCM_FORMAT_FLOAT = SNDRV_PCM_FORMAT_FLOAT_BE,
	SNDRV_PCM_FORMAT_FLOAT64 = SNDRV_PCM_FORMAT_FLOAT64_BE,
	SNDRV_PCM_FORMAT_IEC958_SUBFRAME = SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE,
#endif
};

enum sndrv_pcm_subformat {
	SNDRV_PCM_SUBFORMAT_STD = 0,
	SNDRV_PCM_SUBFORMAT_LAST = SNDRV_PCM_SUBFORMAT_STD,
};

enum sndrv_pcm_state {
	SNDRV_PCM_STATE_OPEN = 0,	/* stream is open */
	SNDRV_PCM_STATE_SETUP,		/* stream has a setup */
	SNDRV_PCM_STATE_PREPARED,	/* stream is ready to start */
	SNDRV_PCM_STATE_RUNNING,	/* stream is running */
	SNDRV_PCM_STATE_XRUN,		/* stream reached an xrun */
	SNDRV_PCM_STATE_DRAINING,	/* stream is draining */
	SNDRV_PCM_STATE_PAUSED,		/* stream is paused */
	SNDRV_PCM_STATE_SUSPENDED,	/* hardware is suspended */
	SNDRV_PCM_STATE_DISCONNECTED,	/* hardware is disconnected */
	SNDRV_PCM_STATE_LAST = SNDRV_PCM_STATE_DISCONNECTED,
};

enum {
	SNDRV_PCM_MMAP_OFFSET_DATA = 0x00000000,
	SNDRV_PCM_MMAP_OFFSET_STATUS = 0x80000000,
	SNDRV_PCM_MMAP_OFFSET_CONTROL = 0x81000000,
};

union sndrv_pcm_sync_id {
	unsigned char id[16];
	unsigned short id16[8];
	unsigned int id32[4];
};

struct sndrv_pcm_info {
	unsigned int device;		/* RO/WR (control): device number */
	unsigned int subdevice;		/* RO/WR (control): subdevice number */
	int stream;			/* RO/WR (control): stream number */
	int card;			/* R: card number */
	unsigned char id[64];		/* ID (user selectable) */
	unsigned char name[80];		/* name of this device */
	unsigned char subname[32];	/* subdevice name */
	int dev_class;			/* SNDRV_PCM_CLASS_* */
	int dev_subclass;		/* SNDRV_PCM_SUBCLASS_* */
	unsigned int subdevices_count;
	unsigned int subdevices_avail;
	union sndrv_pcm_sync_id sync;	/* hardware synchronization ID */
	unsigned char reserved[64];	/* reserved for future... */
};

enum sndrv_pcm_hw_param {
	SNDRV_PCM_HW_PARAM_ACCESS = 0,	/* Access type */
	SNDRV_PCM_HW_PARAM_FIRST_MASK = SNDRV_PCM_HW_PARAM_ACCESS,
	SNDRV_PCM_HW_PARAM_FORMAT,	/* Format */
	SNDRV_PCM_HW_PARAM_SUBFORMAT,	/* Subformat */
	SNDRV_PCM_HW_PARAM_LAST_MASK = SNDRV_PCM_HW_PARAM_SUBFORMAT,

	SNDRV_PCM_HW_PARAM_SAMPLE_BITS = 8, /* Bits per sample */
	SNDRV_PCM_HW_PARAM_FIRST_INTERVAL = SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
	SNDRV_PCM_HW_PARAM_FRAME_BITS,	/* Bits per frame */
	SNDRV_PCM_HW_PARAM_CHANNELS,	/* Channels */
	SNDRV_PCM_HW_PARAM_RATE,	/* Approx rate */
	SNDRV_PCM_HW_PARAM_PERIOD_TIME,	/* Approx distance between interrupts
					   in us */
	SNDRV_PCM_HW_PARAM_PERIOD_SIZE,	/* Approx frames between interrupts */
	SNDRV_PCM_HW_PARAM_PERIOD_BYTES, /* Approx bytes between interrupts */
	SNDRV_PCM_HW_PARAM_PERIODS,	/* Approx interrupts per buffer */
	SNDRV_PCM_HW_PARAM_BUFFER_TIME,	/* Approx duration of buffer in us */
	SNDRV_PCM_HW_PARAM_BUFFER_SIZE,	/* Size of buffer in frames */
	SNDRV_PCM_HW_PARAM_BUFFER_BYTES, /* Size of buffer in bytes */
	SNDRV_PCM_HW_PARAM_TICK_TIME,	/* Approx tick duration in us */
	SNDRV_PCM_HW_PARAM_LAST_INTERVAL = SNDRV_PCM_HW_PARAM_TICK_TIME
};

struct sndrv_interval {
	unsigned int min, max;
	unsigned int openmin:1,
		     openmax:1,
		     integer:1,
		     empty:1;
};

struct sndrv_mask {
	u_int32_t bits[(SNDRV_MASK_MAX+31)/32];
};

struct sndrv_pcm_hw_params {
	unsigned int flags;
	struct sndrv_mask masks[SNDRV_PCM_HW_PARAM_LAST_MASK -
			       SNDRV_PCM_HW_PARAM_FIRST_MASK + 1];
	struct sndrv_mask mres[5];	/* reserved masks */
	struct sndrv_interval intervals[SNDRV_PCM_HW_PARAM_LAST_INTERVAL -
				        SNDRV_PCM_HW_PARAM_FIRST_INTERVAL + 1];
	struct sndrv_interval ires[9];	/* reserved intervals */
	unsigned int rmask;		/* W: requested masks */
	unsigned int cmask;		/* R: changed masks */
	unsigned int info;		/* R: Info flags for returned setup */
	unsigned int msbits;		/* R: used most significant bits */
	unsigned int rate_num;		/* R: rate numerator */
	unsigned int rate_den;		/* R: rate denominator */
	sndrv_pcm_uframes_t fifo_size;	/* R: chip FIFO size in frames */
	unsigned char reserved[64];	/* reserved for future */
};

enum sndrv_pcm_tstamp {
	SNDRV_PCM_TSTAMP_NONE = 0,
	SNDRV_PCM_TSTAMP_MMAP,
	SNDRV_PCM_TSTAMP_LAST = SNDRV_PCM_TSTAMP_MMAP,
};

struct sndrv_pcm_sw_params {
	int tstamp_mode;			/* timestamp mode */
	unsigned int period_step;
	unsigned int sleep_min;			/* min ticks to sleep */
	sndrv_pcm_uframes_t avail_min;		/* min avail frames for wakeup */
	sndrv_pcm_uframes_t xfer_align;		/* xfer size need to be a multiple */
	sndrv_pcm_uframes_t start_threshold;	/* min hw_avail frames for automatic start */
	sndrv_pcm_uframes_t stop_threshold;	/* min avail frames for automatic stop */
	sndrv_pcm_uframes_t silence_threshold;	/* min distance from noise for silence filling */
	sndrv_pcm_uframes_t silence_size;	/* silence block size */
	sndrv_pcm_uframes_t boundary;		/* pointers wrap point */
	unsigned char reserved[64];		/* reserved for future */
};

struct sndrv_pcm_channel_info {
	unsigned int channel;
	long int offset;			/* mmap offset */
	unsigned int first;		/* offset to first sample in bits */
	unsigned int step;		/* samples distance in bits */
};

struct sndrv_pcm_status {
	int state;			/* stream state */
	struct timespec trigger_tstamp;	/* time when stream was started/stopped/paused */
	struct timespec tstamp;		/* reference timestamp */
	sndrv_pcm_uframes_t appl_ptr;	/* appl ptr */
	sndrv_pcm_uframes_t hw_ptr;	/* hw ptr */
	sndrv_pcm_sframes_t delay;	/* current delay in frames */
	sndrv_pcm_uframes_t avail;	/* number of frames available */
	sndrv_pcm_uframes_t avail_max;	/* max frames available on hw since last status */
	sndrv_pcm_uframes_t overrange;	/* count of ADC (capture) overrange detections from last status */
	int suspended_state;		/* suspended stream state */
	unsigned char reserved[60];	/* must be filled with zero */
};

struct sndrv_pcm_mmap_status {
	int state;			/* RO: state - SNDRV_PCM_STATE_XXXX */
	int pad1;			/* Needed for 64 bit alignment */
	sndrv_pcm_uframes_t hw_ptr;	/* RO: hw ptr (0...boundary-1) */
	struct timespec tstamp;		/* Timestamp */
	int suspended_state;		/* RO: suspended stream state */
};

struct sndrv_pcm_mmap_control {
	sndrv_pcm_uframes_t appl_ptr;	/* RW: appl ptr (0...boundary-1) */
	sndrv_pcm_uframes_t avail_min;	/* RW: min available frames for wakeup */
};

struct sndrv_pcm_sync_ptr {
	unsigned int flags;
	union {
		struct sndrv_pcm_mmap_status status;
		unsigned char reserved[64];
	} s;
	union {
		struct sndrv_pcm_mmap_control control;
		unsigned char reserved[64];
	} c;
};

struct sndrv_xferi {
	sndrv_pcm_sframes_t result;
	void *buf;
	sndrv_pcm_uframes_t frames;
};

struct sndrv_xfern {
	sndrv_pcm_sframes_t result;
	void **bufs;
	sndrv_pcm_uframes_t frames;
};

enum sndrv_rawmidi_stream {
	SNDRV_RAWMIDI_STREAM_OUTPUT = 0,
	SNDRV_RAWMIDI_STREAM_INPUT,
	SNDRV_RAWMIDI_STREAM_LAST = SNDRV_RAWMIDI_STREAM_INPUT,
};

struct sndrv_rawmidi_info {
	unsigned int device;		/* RO/WR (control): device number */
	unsigned int subdevice;		/* RO/WR (control): subdevice number */
	int stream;			/* WR: stream */
	int card;			/* R: card number */
	unsigned int flags;		/* SNDRV_RAWMIDI_INFO_XXXX */
	unsigned char id[64];		/* ID (user selectable) */
	unsigned char name[80];		/* name of device */
	unsigned char subname[32];	/* name of active or selected subdevice */
	unsigned int subdevices_count;
	unsigned int subdevices_avail;
	unsigned char reserved[64];	/* reserved for future use */
};

struct sndrv_rawmidi_params {
	int stream;
	size_t buffer_size;		/* queue size in bytes */
	size_t avail_min;		/* minimum avail bytes for wakeup */
	unsigned int no_active_sensing: 1; /* do not send active sensing byte in close() */
	unsigned char reserved[16];	/* reserved for future use */
};

struct sndrv_rawmidi_status {
	int stream;
	struct timespec tstamp;		/* Timestamp */
	size_t avail;			/* available bytes */
	size_t xruns;			/* count of overruns since last status (in bytes) */
	unsigned char reserved[16];	/* reserved for future use */
};

enum sndrv_timer_class {
	SNDRV_TIMER_CLASS_NONE = -1,
	SNDRV_TIMER_CLASS_SLAVE = 0,
	SNDRV_TIMER_CLASS_GLOBAL,
	SNDRV_TIMER_CLASS_CARD,
	SNDRV_TIMER_CLASS_PCM,
	SNDRV_TIMER_CLASS_LAST = SNDRV_TIMER_CLASS_PCM,
};

/* slave timer classes */
enum sndrv_timer_slave_class {
	SNDRV_TIMER_SCLASS_NONE = 0,
	SNDRV_TIMER_SCLASS_APPLICATION,
	SNDRV_TIMER_SCLASS_SEQUENCER,		/* alias */
	SNDRV_TIMER_SCLASS_OSS_SEQUENCER,	/* alias */
	SNDRV_TIMER_SCLASS_LAST = SNDRV_TIMER_SCLASS_OSS_SEQUENCER,
};

struct sndrv_timer_ginfo {
	struct sndrv_timer_id tid;	/* requested timer ID */
	unsigned int flags;		/* timer flags - SNDRV_TIMER_FLG_* */
	int card;			/* card number */
	unsigned char id[64];		/* timer identification */
	unsigned char name[80];		/* timer name */
	unsigned long reserved0;	/* reserved for future use */
	unsigned long resolution;	/* average period resolution in ns */
	unsigned long resolution_min;	/* minimal period resolution in ns */
	unsigned long resolution_max;	/* maximal period resolution in ns */
	unsigned int clients;		/* active timer clients */
	unsigned char reserved[32];
};

struct sndrv_timer_gparams {
	struct sndrv_timer_id tid;	/* requested timer ID */
	unsigned long period_num;	/* requested precise period duration (in seconds) - numerator */
	unsigned long period_den;	/* requested precise period duration (in seconds) - denominator */
	unsigned char reserved[32];
};

struct sndrv_timer_gstatus {
	struct sndrv_timer_id tid;	/* requested timer ID */
	unsigned long resolution;	/* current period resolution in ns */
	unsigned long resolution_num;	/* precise current period resolution (in seconds) - numerator */
	unsigned long resolution_den;	/* precise current period resolution (in seconds) - denominator */
	unsigned char reserved[32];
};

struct sndrv_timer_select {
	struct sndrv_timer_id id;	/* bind to timer ID */
	unsigned char reserved[32];	/* reserved */
};

struct sndrv_timer_info {
	unsigned int flags;		/* timer flags - SNDRV_TIMER_FLG_* */
	int card;			/* card number */
	unsigned char id[64];		/* timer identificator */
	unsigned char name[80];		/* timer name */
	unsigned long reserved0;	/* reserved for future use */
	unsigned long resolution;	/* average period resolution in ns */
	unsigned char reserved[64];	/* reserved */
};

struct sndrv_timer_params {
	unsigned int flags;		/* flags - SNDRV_MIXER_PSFLG_* */
	unsigned int ticks;		/* requested resolution in ticks */
	unsigned int queue_size;	/* total size of queue (32-1024) */
	unsigned int reserved0;		/* reserved, was: failure locations */
	unsigned int filter;		/* event filter (bitmask of SNDRV_TIMER_EVENT_*) */
	unsigned char reserved[60];	/* reserved */
};

struct sndrv_timer_status {
	struct timespec tstamp;		/* Timestamp - last update */
	unsigned int resolution;	/* current period resolution in ns */
	unsigned int lost;		/* counter of master tick lost */
	unsigned int overrun;		/* count of read queue overruns */
	unsigned int queue;		/* used queue size */
	unsigned char reserved[64];	/* reserved */
};

struct sndrv_timer_read {
	unsigned int resolution;
	unsigned int ticks;
};

enum sndrv_timer_event {
	SNDRV_TIMER_EVENT_RESOLUTION = 0,	/* val = resolution in ns */
	SNDRV_TIMER_EVENT_TICK,			/* val = ticks */
	SNDRV_TIMER_EVENT_START,		/* val = resolution in ns */
	SNDRV_TIMER_EVENT_STOP,			/* val = 0 */
	SNDRV_TIMER_EVENT_CONTINUE,		/* val = resolution in ns */
	SNDRV_TIMER_EVENT_PAUSE,		/* val = 0 */
	SNDRV_TIMER_EVENT_EARLY,		/* val = 0, early event */
	SNDRV_TIMER_EVENT_SUSPEND,		/* val = 0 */
	SNDRV_TIMER_EVENT_RESUME,		/* val = resolution in ns */
	/* master timer events for slave timer instances */
	SNDRV_TIMER_EVENT_MSTART = SNDRV_TIMER_EVENT_START + 10,
	SNDRV_TIMER_EVENT_MSTOP = SNDRV_TIMER_EVENT_STOP + 10,
	SNDRV_TIMER_EVENT_MCONTINUE = SNDRV_TIMER_EVENT_CONTINUE + 10,
	SNDRV_TIMER_EVENT_MPAUSE = SNDRV_TIMER_EVENT_PAUSE + 10,
	SNDRV_TIMER_EVENT_MSUSPEND = SNDRV_TIMER_EVENT_SUSPEND + 10,
	SNDRV_TIMER_EVENT_MRESUME = SNDRV_TIMER_EVENT_RESUME + 10,
};

struct sndrv_timer_tread {
	int event;
	struct timespec tstamp;
	unsigned int val;
};

struct sndrv_ctl_card_info {
	int card;			/* card number */
	int pad;			/* reserved for future (was type) */
	unsigned char id[16];		/* ID of card (user selectable) */
	unsigned char driver[16];	/* Driver name */
	unsigned char name[32];		/* Short name of soundcard */
	unsigned char longname[80];	/* name + info text about soundcard */
	unsigned char reserved_[16];	/* reserved for future (was ID of mixer) */
	unsigned char mixername[80];	/* visual mixer identification */
	unsigned char components[80];	/* card components / fine identification, delimited with one space (AC97 etc..) */
	unsigned char reserved[48];	/* reserved for future */
};

enum sndrv_ctl_elem_type {
	SNDRV_CTL_ELEM_TYPE_NONE = 0,		/* invalid */
	SNDRV_CTL_ELEM_TYPE_BOOLEAN,		/* boolean type */
	SNDRV_CTL_ELEM_TYPE_INTEGER,		/* integer type */
	SNDRV_CTL_ELEM_TYPE_ENUMERATED,		/* enumerated type */
	SNDRV_CTL_ELEM_TYPE_BYTES,		/* byte array */
	SNDRV_CTL_ELEM_TYPE_IEC958,		/* IEC958 (S/PDIF) setup */
	SNDRV_CTL_ELEM_TYPE_INTEGER64,		/* 64-bit integer type */
	SNDRV_CTL_ELEM_TYPE_LAST = SNDRV_CTL_ELEM_TYPE_INTEGER64,
};

enum sndrv_ctl_elem_iface {
	SNDRV_CTL_ELEM_IFACE_CARD = 0,		/* global control */
	SNDRV_CTL_ELEM_IFACE_HWDEP,		/* hardware dependent device */
	SNDRV_CTL_ELEM_IFACE_MIXER,		/* virtual mixer device */
	SNDRV_CTL_ELEM_IFACE_PCM,		/* PCM device */
	SNDRV_CTL_ELEM_IFACE_RAWMIDI,		/* RawMidi device */
	SNDRV_CTL_ELEM_IFACE_TIMER,		/* timer device */
	SNDRV_CTL_ELEM_IFACE_SEQUENCER,		/* sequencer client */
	SNDRV_CTL_ELEM_IFACE_LAST = SNDRV_CTL_ELEM_IFACE_SEQUENCER,
};

struct sndrv_ctl_elem_id {
	unsigned int numid;		/* numeric identifier, zero = invalid */
	int iface;			/* interface identifier */
	unsigned int device;		/* device/client number */
	unsigned int subdevice;		/* subdevice (substream) number */
        unsigned char name[44];		/* ASCII name of item */
	unsigned int index;		/* index of item */
};

struct sndrv_ctl_elem_list {
	unsigned int offset;		/* W: first element ID to get */
	unsigned int space;		/* W: count of element IDs to get */
	unsigned int used;		/* R: count of element IDs set */
	unsigned int count;		/* R: count of all elements */
	struct sndrv_ctl_elem_id *pids; /* R: IDs */
	unsigned char reserved[50];
};

struct sndrv_ctl_elem_info {
	struct sndrv_ctl_elem_id id;	/* W: element ID */
	int type;			/* R: value type - SNDRV_CTL_ELEM_TYPE_* */
	unsigned int access;		/* R: value access (bitmask) - SNDRV_CTL_ELEM_ACCESS_* */
	unsigned int count;		/* count of values */
	pid_t owner;			/* owner's PID of this control */
	union {
		struct {
			long min;		/* R: minimum value */
			long max;		/* R: maximum value */
			long step;		/* R: step (0 variable) */
		} integer;
		struct {
			long long min;		/* R: minimum value */
			long long max;		/* R: maximum value */
			long long step;		/* R: step (0 variable) */
		} integer64;
		struct {
			unsigned int items;	/* R: number of items */
			unsigned int item;	/* W: item number */
			char name[64];		/* R: value name */
		} enumerated;
		unsigned char reserved[128];
	} value;
	union {
		unsigned short d[4];		/* dimensions */
		unsigned short *d_ptr;		/* indirect */
	} dimen;
	unsigned char reserved[64-4*sizeof(unsigned short)];
};

struct sndrv_ctl_elem_value {
	struct sndrv_ctl_elem_id id;	/* W: element ID */
	unsigned int indirect: 1;	/* W: use indirect pointer (xxx_ptr member) */
        union {
		union {
			long value[128];
			long *value_ptr;
		} integer;
		union {
			long long value[64];
			long long *value_ptr;
		} integer64;
		union {
			unsigned int item[128];
			unsigned int *item_ptr;
		} enumerated;
		union {
			unsigned char data[512];
			unsigned char *data_ptr;
		} bytes;
		struct sndrv_aes_iec958 iec958;
        } value;                /* RO */
	struct timespec tstamp;
        unsigned char reserved[128-sizeof(struct timespec)];
};

struct sndrv_ctl_tlv {
	unsigned int numid;     /* control element numeric identification */
        unsigned int length;    /* in bytes aligned to 4 */
        unsigned int tlv[0];    /* first TLV */
};

enum sndrv_ctl_event_type {
	SNDRV_CTL_EVENT_ELEM = 0,
	SNDRV_CTL_EVENT_LAST = SNDRV_CTL_EVENT_ELEM,
};

struct sndrv_ctl_event {
	int type;				/* event type - SNDRV_CTL_EVENT_* */
	union {
		struct {
			unsigned int mask;
			struct sndrv_ctl_elem_id id;
		} elem;
                unsigned char data8[60];
        } data;
};

struct sndrv_xferv {
	const struct iovec *vector;
	unsigned long count;
};

typedef struct {
	unsigned int internal_tram_size;	/* in samples */
	unsigned int external_tram_size;	/* in samples */
	char fxbus_names[16][32];		/* names of FXBUSes */
	char extin_names[16][32];		/* names of external inputs */
	char extout_names[32][32];		/* names of external outputs */
	unsigned int gpr_controls;		/* count of GPR controls */
} emu10k1_fx8010_info_t;

enum emu10k1_ctl_elem_iface {
	EMU10K1_CTL_ELEM_IFACE_MIXER = 2,	/* virtual mixer device */
	EMU10K1_CTL_ELEM_IFACE_PCM = 3,		/* PCM device */
};

typedef struct {
	unsigned int pad;		/* don't use */
	int iface;			/* interface identifier */
	unsigned int device;		/* device/client number */
	unsigned int subdevice;		/* subdevice (substream) number */
	unsigned char name[44];		/* ASCII name of item */
	unsigned int index;		/* index of item */
} emu10k1_ctl_elem_id_t;

typedef struct {
	emu10k1_ctl_elem_id_t id;	/* full control ID definition */
	unsigned int vcount;		/* visible count */
	unsigned int count;		/* count of GPR (1..16) */
	unsigned short gpr[32];		/* GPR number(s) */
	unsigned int value[32];		/* initial values */
	unsigned int min;		/* minimum range */
	unsigned int max;		/* maximum range */
	unsigned int translation;	/* translation type (EMU10K1_GPR_TRANSLATION*) */
	unsigned int *tlv;
} emu10k1_fx8010_control_gpr_t;

typedef struct {
	char name[128];

	unsigned long gpr_valid[0x200/(sizeof(unsigned long)*8)]; /* bitmask of valid initializers */
	uint32_t *gpr_map;		  /* initializers */

	unsigned int gpr_add_control_count; /* count of GPR controls to add/replace */
	emu10k1_fx8010_control_gpr_t *gpr_add_controls; /* GPR controls to add/replace */

	unsigned int gpr_del_control_count; /* count of GPR controls to remove */
	emu10k1_ctl_elem_id_t *gpr_del_controls; /* IDs of GPR controls to remove */

	unsigned int gpr_list_control_count; /* count of GPR controls to list */
	unsigned int gpr_list_control_total; /* total count of GPR controls */
	emu10k1_fx8010_control_gpr_t *gpr_list_controls; /* listed GPR controls */

	unsigned long tram_valid[0x100/(sizeof(unsigned long)*8)]; /* bitmask of valid initializers */
	uint32_t *tram_data_map;	/* data initializers */
	uint32_t *tram_addr_map;	/* map initializers */

	unsigned long code_valid[1024/(sizeof(unsigned long)*8)];  /* bitmask of valid instructions */
	uint32_t *code;			/* one instruction - 64 bits */
} emu10k1_fx8010_code_t;

typedef struct {
	unsigned int address;		/* 31.bit == 1 -> external TRAM */
	unsigned int size;		/* size in samples (4 bytes) */
	unsigned int *samples;		/* pointer to samples (20-bit) */
					/* NULL->clear memory */
} emu10k1_fx8010_tram_t;

typedef struct {
	unsigned int substream;		/* substream number */
	unsigned int res1;		/* reserved */
	unsigned int channels;		/* 16-bit channels count, zero = remove this substream */
	unsigned int tram_start;	/* ring buffer position in TRAM (in samples) */
	unsigned int buffer_size;	/* count of buffered samples */
	unsigned short gpr_size;		/* GPR containing size of ringbuffer in samples (host) */
	unsigned short gpr_ptr;		/* GPR containing current pointer in the ring buffer (host = reset, FX8010) */
	unsigned short gpr_count;	/* GPR containing count of samples between two interrupts (host) */
	unsigned short gpr_tmpcount;	/* GPR containing current count of samples to interrupt (host = set, FX8010) */
	unsigned short gpr_trigger;	/* GPR containing trigger (activate) information (host) */
	unsigned short gpr_running;	/* GPR containing info if PCM is running (FX8010) */
	unsigned char pad;		/* reserved */
	unsigned char etram[32];	/* external TRAM address & data (one per channel) */
	unsigned int res2;		/* reserved */
} emu10k1_fx8010_pcm_t;

typedef enum {
	Digiface,
	Multiface,
	H9652,
	H9632,
	Undefined,
} HDSP_IO_Type;

typedef struct _snd_hdsp_peak_rms hdsp_peak_rms_t;

struct _snd_hdsp_peak_rms {
	uint32_t input_peaks[26];
	uint32_t playback_peaks[26];
	uint32_t output_peaks[28];
	uint64_t input_rms[26];
	uint64_t playback_rms[26];
	/* These are only used for H96xx cards */
	uint64_t output_rms[26];
};

typedef struct _snd_hdsp_config_info hdsp_config_info_t;

struct _snd_hdsp_config_info {
	unsigned char pref_sync_ref;
	unsigned char wordclock_sync_check;
	unsigned char spdif_sync_check;
	unsigned char adatsync_sync_check;
	unsigned char adat_sync_check[3];
	unsigned char spdif_in;
	unsigned char spdif_out;
	unsigned char spdif_professional;
	unsigned char spdif_emphasis;
	unsigned char spdif_nonaudio;
	unsigned int spdif_sample_rate;
	unsigned int system_sample_rate;
	unsigned int autosync_sample_rate;
	unsigned char system_clock_mode;
	unsigned char clock_source;
	unsigned char autosync_ref;
	unsigned char line_out;
	unsigned char passthru;
	unsigned char da_gain;
	unsigned char ad_gain;
	unsigned char phone_gain;
	unsigned char xlr_breakout_cable;
	unsigned char analog_extension_board;
};

typedef struct _snd_hdsp_firmware hdsp_firmware_t;

struct _snd_hdsp_firmware {
	void *firmware_data;	/* 24413 x 4 bytes */
};

typedef struct _snd_hdsp_version hdsp_version_t;

struct _snd_hdsp_version {
	HDSP_IO_Type io_type;
	unsigned short firmware_rev;
};

typedef struct _snd_hdsp_mixer hdsp_mixer_t;

struct _snd_hdsp_mixer {
	unsigned short matrix[HDSP_MATRIX_MIXER_SIZE];
};

typedef struct _snd_hdsp_9632_aeb hdsp_9632_aeb_t;

struct _snd_hdsp_9632_aeb {
	int aebi;
	int aebo;
};

typedef struct snd_sb_csp_mc_header {
	char codec_name[16];		/* id name of codec */
	unsigned short func_req;	/* requested function */
} snd_sb_csp_mc_header_t;

typedef struct snd_sb_csp_microcode {
	snd_sb_csp_mc_header_t info;
	unsigned char data[SNDRV_SB_CSP_MAX_MICROCODE_FILE_SIZE];
} snd_sb_csp_microcode_t;

typedef struct snd_sb_csp_start {
	int sample_width;	/* sample width, look above */
	int channels;		/* channels, look above */
} snd_sb_csp_start_t;

typedef struct snd_sb_csp_info {
	char codec_name[16];		/* id name of codec */
	unsigned short func_nr;		/* function number */
	unsigned int acc_format;	/* accepted PCM formats */
	unsigned short acc_channels;	/* accepted channels */
	unsigned short acc_width;	/* accepted sample width */
	unsigned short acc_rates;	/* accepted sample rates */
	unsigned short csp_mode;	/* CSP mode, see above */
	unsigned short run_channels;	/* current channels  */
	unsigned short run_width;	/* current sample width */
	unsigned short version;		/* version id: 0x10 - 0x1f */
	unsigned short state;		/* state bits */
} snd_sb_csp_info_t;

struct sscape_bootblock
{
  unsigned char code[256];
  unsigned version;
};

struct sscape_microcode
{
  unsigned char *code;
};
