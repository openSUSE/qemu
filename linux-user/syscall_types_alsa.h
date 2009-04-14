/*
 *  Advanced Linux Sound Architecture
 *
 *   This program is free software, you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY, without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   aTYPE_LONG, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

STRUCT (sndrv_pcm_sframes, TYPE_LONG)
STRUCT (sndrv_seq_event_type, TYPE_CHAR)
STRUCT (sndrv_seq_instr_cluster, TYPE_INT)
STRUCT (sndrv_seq_position, TYPE_INT)
STRUCT (sndrv_seq_frequency, TYPE_INT)
STRUCT (sndrv_seq_tick_time, TYPE_INT)
STRUCT (sndrv_seq_instr_size, TYPE_INT)
STRUCT (sndrv_pcm_uframes, TYPE_ULONG)


STRUCT (timespec,
	TYPE_LONG,
	TYPE_LONG
       )

STRUCT( fm_operator,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT(fm_instrument,
	MK_ARRAY(TYPE_INT, 4),	/* share id - zero = no sharing */
	TYPE_CHAR,		/* instrument type */

	MK_ARRAY(MK_STRUCT(STRUCT_fm_operator), 4),
	MK_ARRAY(TYPE_CHAR, 2),

	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( fm_xoperator,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( fm_xinstrument,
	TYPE_INT,			/* structure type */

	MK_ARRAY(TYPE_INT, 4),		/* share id - zero = no sharing */
	TYPE_CHAR,			/* instrument type */

	MK_ARRAY(MK_STRUCT(STRUCT_fm_xoperator), 4),		/* fm operators */
	MK_ARRAY(TYPE_CHAR, 2),

	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( gf1_wave,
	MK_ARRAY(TYPE_INT, 4),		/* share id - zero = no sharing */
	TYPE_INT,			/* wave format */

	TYPE_INT,			/* some other ID for this instrument */
	TYPE_INT,			/* begin of waveform in onboard memory */
	TYPE_PTRVOID,			/* poTYPE_INTer to waveform in system memory */

	TYPE_INT,			/* size of waveform in samples */
	TYPE_INT,			/* start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,		/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,		/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_SHORT,	/* loop repeat - 0 = forever */

	TYPE_CHAR,		/* GF1 patch flags */
	TYPE_CHAR,
	TYPE_INT,	/* sample rate in Hz */
	TYPE_INT,	/* low frequency range */
	TYPE_INT,	/* high frequency range */
	TYPE_INT,	/* root frequency range */
	TYPE_SHORT,
	TYPE_CHAR,
	MK_ARRAY(TYPE_CHAR, 6),
	MK_ARRAY(TYPE_CHAR, 6),
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_SHORT,
	TYPE_SHORT,	/* 0-2048 or 0-2 */

	TYPE_PTRVOID
)

STRUCT(gf1_instrument,
	TYPE_SHORT,
	TYPE_SHORT,	/* 0 - none, 1-65535 */

	TYPE_CHAR,		/* effect 1 */
	TYPE_CHAR,	/* 0-127 */
	TYPE_CHAR,		/* effect 2 */
	TYPE_CHAR,	/* 0-127 */

	TYPE_PTRVOID		/* first waveform */
)

STRUCT( gf1_xwave,
	TYPE_INT,			/* structure type */

	MK_ARRAY(TYPE_INT, 4),		/* share id - zero = no sharing */
	TYPE_INT,			/* wave format */

	TYPE_INT,			/* size of waveform in samples */
	TYPE_INT,			/* start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,		/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,			/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_SHORT,		/* loop repeat - 0 = forever */

	TYPE_CHAR,			/* GF1 patch flags */
	TYPE_CHAR,
	TYPE_INT,		/* sample rate in Hz */
	TYPE_INT,		/* low frequency range */
	TYPE_INT,		/* high frequency range */
	TYPE_INT,		/* root frequency range */
	TYPE_SHORT,
	TYPE_CHAR,
	MK_ARRAY(TYPE_CHAR, 6),
	MK_ARRAY(TYPE_CHAR, 6),
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_SHORT,
	TYPE_SHORT		/* 0-2048 or 0-2 */
)

STRUCT( gf1_xinstrument,
	TYPE_INT,

	TYPE_SHORT,
	TYPE_SHORT,		/* 0 - none, 1-65535 */

	TYPE_CHAR,		/* effect 1 */
	TYPE_CHAR,		/* 0-127 */
	TYPE_CHAR,		/* effect 2 */
	TYPE_CHAR		/* 0-127 */
)

STRUCT( gf1_info,
	TYPE_CHAR,		/* supported wave flags */
	MK_ARRAY(TYPE_CHAR, 3),
	TYPE_INT,		/* supported features */
	TYPE_INT,		/* maximum 8-bit wave length */
	TYPE_INT		/* maximum 16-bit wave length */
)

STRUCT( iwffff_wave,
	MK_ARRAY(TYPE_INT, 4),		/* share id - zero = no sharing */
	TYPE_INT,		/* wave format */

	TYPE_INT,	/* some other ID for this wave */
	TYPE_INT,	/* begin of waveform in onboard memory */
	TYPE_PTRVOID,	/* poTYPE_INTer to waveform in system memory */

	TYPE_INT,		/* size of waveform in samples */
	TYPE_INT,		/* start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,	/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,		/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_SHORT,	/* loop repeat - 0 = forever */
	TYPE_INT,	/* sample ratio (44100 * 1024 / rate) */
	TYPE_CHAR,	/* 0 - 127 (no corresponding midi controller) */
	TYPE_CHAR,		/* lower frequency range for this waveform */
	TYPE_CHAR,	/* higher frequency range for this waveform */
	TYPE_CHAR,

	TYPE_PTRVOID
)

STRUCT( iwffff_lfo,
	TYPE_SHORT,		/* (0-2047) 0.01Hz - 21.5Hz */
	TYPE_SHORT,		/* volume +- (0-255) 0.48675dB/step */
	TYPE_SHORT,		/* 0 - 950 deciseconds */
	TYPE_CHAR,		/* see to IWFFFF_LFO_SHAPE_XXXX */
	TYPE_CHAR		/* 0 - 255 deciseconds */
)

STRUCT( iwffff_env_point,
	TYPE_SHORT,
	TYPE_SHORT
)

STRUCT( iwffff_env_record,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_PTRVOID
)

STRUCT( iwffff_env,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_PTRVOID // MK_STRUCT(STRUCT_iwffff_env_record)
)

STRUCT( iwffff_layer,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,	/* range for layer based */
	TYPE_CHAR,	/* on either velocity or frequency */
	TYPE_CHAR,		/* pan offset from CC1 (0 left - 127 right) */
	TYPE_CHAR,	/* position based on frequency (0-127) */
	TYPE_CHAR,	/* 0-127 (no corresponding midi controller) */
	MK_STRUCT(STRUCT_iwffff_lfo),		/* tremolo effect */
	MK_STRUCT(STRUCT_iwffff_lfo),		/* vibrato effect */
	TYPE_SHORT,	/* 0-2048, 1024 is equal to semitone scaling */
	TYPE_CHAR,	/* center for keyboard frequency scaling */
	TYPE_CHAR,
	MK_STRUCT(STRUCT_iwffff_env),		/* pitch envelope */
	MK_STRUCT(STRUCT_iwffff_env),		/* volume envelope */

	TYPE_PTRVOID, // iwffff_wave_t *wave,
	TYPE_PTRVOID  // MK_STRUCT(STRUCT_iwffff_layer)
)

STRUCT(iwffff_instrument,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,	/* 0 - none, 1-65535 */

	TYPE_CHAR,		/* effect 1 */
	TYPE_CHAR,	/* 0-127 */
	TYPE_CHAR,		/* effect 2 */
	TYPE_CHAR,	/* 0-127 */

	TYPE_PTRVOID  // iwffff_layer_t *layer,		/* first layer */
)

STRUCT( iwffff_xwave,
	TYPE_INT,			/* structure type */

	MK_ARRAY(TYPE_INT, 4),		/* share id - zero = no sharing */

	TYPE_INT,			/* wave format */
	TYPE_INT,			/* offset to ROM (address) */

	TYPE_INT,			/* size of waveform in samples */
	TYPE_INT,			/* start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,		/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,			/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_SHORT,		/* loop repeat - 0 = forever */
	TYPE_INT,		/* sample ratio (44100 * 1024 / rate) */
	TYPE_CHAR,		/* 0 - 127 (no corresponding midi controller) */
	TYPE_CHAR,			/* lower frequency range for this waveform */
	TYPE_CHAR,			/* higher frequency range for this waveform */
	TYPE_CHAR
)

STRUCT( iwffff_xlfo,
	TYPE_SHORT,			/* (0-2047) 0.01Hz - 21.5Hz */
	TYPE_SHORT,			/* volume +- (0-255) 0.48675dB/step */
	TYPE_SHORT,			/* 0 - 950 deciseconds */
	TYPE_CHAR,			/* see to ULTRA_IW_LFO_SHAPE_XXXX */
	TYPE_CHAR			/* 0 - 255 deciseconds */
)

STRUCT( iwffff_xenv_point,
	TYPE_SHORT,
	TYPE_SHORT
)

STRUCT( iwffff_xenv_record,
	TYPE_INT,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( iwffff_xenv,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( iwffff_xlayer,
	TYPE_INT,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,			/* range for layer based */
	TYPE_CHAR,		/* on either velocity or frequency */
	TYPE_CHAR,			/* pan offset from CC1 (0 left - 127 right) */
	TYPE_CHAR,		/* position based on frequency (0-127) */
	TYPE_CHAR,		/* 0-127 (no corresponding midi controller) */
	MK_STRUCT(STRUCT_iwffff_xlfo),		/* tremolo effect */
	MK_STRUCT(STRUCT_iwffff_xlfo),		/* vibrato effect */
	TYPE_SHORT,		/* 0-2048, 1024 is equal to semitone scaling */
	TYPE_CHAR,		/* center for keyboard frequency scaling */
	TYPE_CHAR,
	MK_STRUCT(STRUCT_iwffff_xenv),		/* pitch envelope */
	MK_STRUCT(STRUCT_iwffff_xenv)		/* volume envelope */
)

STRUCT( iwffff_xinstrument,
	TYPE_INT,

	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_SHORT,		/* 0 - none, 1-65535 */

	TYPE_CHAR,			/* effect 1 */
	TYPE_CHAR,		/* 0-127 */
	TYPE_CHAR,			/* effect 2 */
	TYPE_CHAR			/* 0-127 */
)

STRUCT(iwffff_rom_header,
	MK_ARRAY(TYPE_CHAR, 8),
	TYPE_CHAR,
	TYPE_CHAR,
	MK_ARRAY(TYPE_CHAR, 16),
	MK_ARRAY(TYPE_CHAR, 10),
	TYPE_SHORT,
	TYPE_SHORT,
	TYPE_INT,
	MK_ARRAY(TYPE_CHAR, 128),
	MK_ARRAY(TYPE_CHAR, 64),
	MK_ARRAY(TYPE_CHAR, 128)
)

STRUCT( iwffff_info,
	TYPE_INT,		/* supported format bits */
	TYPE_INT,		/* supported effects (1 << IWFFFF_EFFECT*) */
	TYPE_INT,		/* LFO effects */
	TYPE_INT,		/* maximum 8-bit wave length */
	TYPE_INT		/* maximum 16-bit wave length */
)

STRUCT( simple_instrument_info,
	TYPE_INT,		/* supported format bits */
	TYPE_INT,		/* supported effects (1 << SIMPLE_EFFECT_*) */
	TYPE_INT,		/* maximum 8-bit wave length */
	TYPE_INT		/* maximum 16-bit wave length */
)

STRUCT(simple_instrument,
	MK_ARRAY(TYPE_INT, 4),	/* share id - zero = no sharing */
	TYPE_INT,		/* wave format */

	TYPE_INT,	/* some other ID for this instrument */
	TYPE_INT,	/* begin of waveform in onboard memory */
	TYPE_PTRVOID,		/* poTYPE_INTer to waveform in system memory */

	TYPE_INT,		/* size of waveform in samples */
	TYPE_INT,		/* start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,	/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,		/* loop end offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_SHORT,	/* loop repeat - 0 = forever */

	TYPE_CHAR,		/* effect 1 */
	TYPE_CHAR,	/* 0-127 */
	TYPE_CHAR,		/* effect 2 */
	TYPE_CHAR	/* 0-127 */
)

STRUCT( simple_xinstrument,
	TYPE_INT,

	MK_ARRAY(TYPE_INT, 4),		/* share id - zero = no sharing */
	TYPE_INT,			/* wave format */

	TYPE_INT,			/* size of waveform in samples */
	TYPE_INT,			/* start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,			/* bits loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_INT,			/* loop start offset in samples * 16 (lowest 4 bits - fraction) */
	TYPE_SHORT,			/* loop repeat - 0 = forever */

	TYPE_CHAR,			/* effect 1 */
	TYPE_CHAR,			/* 0-127 */
	TYPE_CHAR,			/* effect 2 */
	TYPE_CHAR			/* 0-127 */
)

/** event address */
STRUCT( sndrv_seq_addr,
	TYPE_CHAR,	/**< Client number:         0..255, 255 = broadcast to all clients */
	TYPE_CHAR	/**< Port within client:    0..255, 255 = broadcast to all ports */
)

/** port connection */
STRUCT( sndrv_seq_connect,
	MK_STRUCT(STRUCT_sndrv_seq_addr),
	MK_STRUCT(STRUCT_sndrv_seq_addr)
)

STRUCT( sndrv_seq_ev_note,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,	/* only for SNDRV_SEQ_EVENT_NOTE */
	TYPE_INT		/* only for SNDRV_SEQ_EVENT_NOTE */
)

	/* controller event */
STRUCT( sndrv_seq_ev_ctrl,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,	/* pad */
	TYPE_INT,
	TYPE_INT
)

	/* generic set of bytes (12x8 bit) */
STRUCT( sndrv_seq_ev_raw8,
	MK_ARRAY(TYPE_CHAR, 12)	/* 8 bit value */
)

	/* generic set of TYPE_INTegers (3x32 bit) */
STRUCT( sndrv_seq_ev_raw32,
	MK_ARRAY(TYPE_INT, 3)	/* 32 bit value */
)

	/* external stored data */
STRUCT( sndrv_seq_ev_ext,
	TYPE_INT,	/* length of data */
	TYPE_PTRVOID		/* poTYPE_INTer to data (note: maybe 64-bit) */
)

/* Instrument type */
STRUCT( sndrv_seq_instr,
	TYPE_INT,
	TYPE_INT,		/* the upper byte means a private instrument (owner - client #) */
	TYPE_SHORT,
	TYPE_SHORT
)

	/* sample number */
STRUCT( sndrv_seq_ev_sample,
	TYPE_INT,
	TYPE_SHORT,
	TYPE_SHORT
)

	/* sample cluster */
STRUCT( sndrv_seq_ev_cluster,
	TYPE_INT
)

	/* sample volume control, if any value is set to -1 == do not change */
STRUCT( sndrv_seq_ev_volume,
	TYPE_SHORT,	/* range: 0-16383 */
	TYPE_SHORT,	/* left-right balance, range: 0-16383 */
	TYPE_SHORT,	/* front-rear balance, range: 0-16383 */
	TYPE_SHORT	/* down-up balance, range: 0-16383 */
)

	/* simple loop redefinition */
STRUCT( sndrv_seq_ev_loop,
	TYPE_INT,	/* loop start (in samples) * 16 */
	TYPE_INT	/* loop end (in samples) * 16 */
)

STRUCT( sndrv_seq_ev_sample_control,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,	/* pad */
	MK_ARRAY(TYPE_INT, 2)
)



/* INSTR_BEGIN event */
STRUCT( sndrv_seq_ev_instr_begin,
	TYPE_INT
)

STRUCT( sndrv_seq_result,
	TYPE_INT,
	TYPE_INT
)


STRUCT( sndrv_seq_real_time,
	TYPE_INT,
	TYPE_INT
)

STRUCT( sndrv_seq_queue_skew,
	TYPE_INT,
	TYPE_INT
)

	/* queue timer control */
STRUCT( sndrv_seq_ev_queue_control,
	TYPE_CHAR,			/* affected queue */
	MK_ARRAY(TYPE_CHAR, 3),			/* reserved */
	MK_ARRAY(TYPE_INT, 2)
)

	/* quoted event - inside the kernel only */
STRUCT( sndrv_seq_ev_quote,
	MK_STRUCT(STRUCT_sndrv_seq_addr),		/* original sender */
	TYPE_SHORT,		/* optional data */
	MK_STRUCT(STRUCT_sndrv_seq_event)		/* quoted event */
)


	/* sequencer event */
STRUCT( sndrv_seq_event,
	TYPE_CHAR,	/* event type */
	TYPE_CHAR,		/* event flags */
	TYPE_CHAR,

	TYPE_CHAR,		/* schedule queue */
	MK_STRUCT(STRUCT_sndrv_seq_real_time),	/* schedule time */


	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* source address */
	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* destination address */

	MK_ARRAY(TYPE_INT,3)
)


/*
 * bounce event - stored as variable size data
 */
STRUCT( sndrv_seq_event_bounce,
	TYPE_INT,
	MK_STRUCT(STRUCT_sndrv_seq_event)
	/* external data follows here. */
)

STRUCT( sndrv_seq_system_info,
	TYPE_INT,			/* maximum queues count */
	TYPE_INT,			/* maximum clients count */
	TYPE_INT,			/* maximum ports per client */
	TYPE_INT,			/* maximum channels per port */
	TYPE_INT,		/* current clients */
	TYPE_INT,			/* current queues */
	MK_ARRAY(TYPE_CHAR, 24)
)

STRUCT( sndrv_seq_running_info,
	TYPE_CHAR,		/* client id */
	TYPE_CHAR,	/* 1 = big-endian */
	TYPE_CHAR,
	TYPE_CHAR,		/* reserved */
	MK_ARRAY(TYPE_CHAR, 12)
)

STRUCT( sndrv_seq_client_info,
	TYPE_INT,			/* client number to inquire */
	TYPE_INT,			/* client type */
	MK_ARRAY(TYPE_CHAR, 64),			/* client name */
	TYPE_INT,		/* filter flags */
	MK_ARRAY(TYPE_CHAR, 8), /* multicast filter bitmap */
	MK_ARRAY(TYPE_CHAR, 32),	/* event filter bitmap */
	TYPE_INT,			/* RO: number of ports */
	TYPE_INT,			/* number of lost events */
	MK_ARRAY(TYPE_CHAR, 64)		/* for future use */
)

STRUCT( sndrv_seq_client_pool,
	TYPE_INT,			/* client number to inquire */
	TYPE_INT,		/* outgoing (write) pool size */
	TYPE_INT,			/* incoming (read) pool size */
	TYPE_INT,		/* minimum free pool size for select/blocking mode */
	TYPE_INT,		/* unused size */
	TYPE_INT,			/* unused size */
	MK_ARRAY(TYPE_CHAR, 64)
)

STRUCT( sndrv_seq_remove_events,
	TYPE_INT,	/* Flags that determine what gets removed */

	MK_STRUCT(STRUCT_sndrv_seq_real_time),

	TYPE_CHAR,	/* Queue for REMOVE_DEST */
	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* Address for REMOVE_DEST */
	TYPE_CHAR,	/* Channel for REMOVE_DEST */

	TYPE_INT,	/* For REMOVE_EVENT_TYPE */
	TYPE_CHAR,	/* Tag for REMOVE_TAG */

	MK_ARRAY(TYPE_INT, 10)	/* To allow for future binary compatibility */

)

STRUCT( sndrv_seq_port_info,
	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* client/port numbers */
	MK_ARRAY(TYPE_CHAR, 64),			/* port name */

	TYPE_INT,	/* port capability bits */
	TYPE_INT,		/* port type bits */
	TYPE_INT,		/* channels per MIDI port */
	TYPE_INT,		/* voices per MIDI port */
	TYPE_INT,		/* voices per SYNTH port */

	TYPE_INT,			/* R/O: subscribers for output (from this port) */
	TYPE_INT,			/* R/O: subscribers for input (to this port) */

	TYPE_PTRVOID,			/* reserved for kernel use (must be NULL) */
	TYPE_INT,		/* misc. conditioning */
	TYPE_CHAR,	/* queue # for timestamping */
	MK_ARRAY(TYPE_CHAR, 59)		/* for future use */
)

STRUCT( sndrv_seq_queue_info,
	TYPE_INT,		/* queue id */

	/*
	 *  security settings, only owner of this queue can start/stop timer
	 *  etc. if the queue is locked for other clients
	 */
	TYPE_INT,		/* client id for owner of the queue */
	TYPE_INT,		/* timing queue locked for other queues */
	MK_ARRAY(TYPE_CHAR, 64),		/* name of this queue */
	TYPE_INT,	/* flags */
	MK_ARRAY(TYPE_CHAR, 60)	/* for future use */

)

STRUCT( sndrv_seq_queue_status,
	TYPE_INT,			/* queue id */
	TYPE_INT,			/* read-only - queue size */
	TYPE_INT,	/* current tick */
	MK_STRUCT(STRUCT_sndrv_seq_real_time), /* current time */
	TYPE_INT,			/* running state of queue */
	TYPE_INT,			/* various flags */
	MK_ARRAY(TYPE_CHAR, 64)		/* for the future */
)

STRUCT( sndrv_seq_queue_tempo,
	TYPE_INT,			/* sequencer queue */
	TYPE_INT,
	TYPE_INT,
	TYPE_INT,	/* queue skew */
	TYPE_INT,		/* queue skew base */
	MK_ARRAY(TYPE_CHAR, 24)		/* for the future */
)

STRUCT( sndrv_timer_id,
	TYPE_INT,
	TYPE_INT,
	TYPE_INT,
	TYPE_INT,
	TYPE_INT
)

STRUCT( sndrv_seq_queue_timer,
	TYPE_INT,			/* sequencer queue */
	TYPE_INT,			/* source timer type */
	MK_STRUCT(STRUCT_sndrv_timer_id),	/* ALSA's timer ID */
	TYPE_INT,	/* resolution in Hz */
	MK_ARRAY(TYPE_CHAR, 64)		/* for the future use */
)

STRUCT( sndrv_seq_queue_client,
	TYPE_INT,		/* sequencer queue */
	TYPE_INT,		/* sequencer client */
	TYPE_INT,		/* queue is used with this client
				   (must be set for accepting events) */
	/* per client watermarks */
	MK_ARRAY(TYPE_CHAR, 64)	/* for future use */
)

STRUCT( sndrv_seq_port_subscribe,
	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* sender address */
	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* destination address */
	TYPE_INT,		/* number of voices to be allocated (0 = don't care) */
	TYPE_INT,		/* modes */
	TYPE_CHAR,		/* input time-stamp queue (optional) */
	MK_ARRAY(TYPE_CHAR, 3),		/* reserved */
	MK_ARRAY(TYPE_CHAR, 64)
)

STRUCT( sndrv_seq_query_subs,
	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* client/port id to be searched */
	TYPE_INT,		/* READ or WRITE */
	TYPE_INT,		/* 0..N-1 */
	TYPE_INT,		/* R/O: number of subscriptions on this port */
	MK_STRUCT(STRUCT_sndrv_seq_addr),	/* R/O: result */
	TYPE_CHAR,	/* R/O: result */
	TYPE_INT,	/* R/O: result */
	MK_ARRAY(TYPE_CHAR, 64)	/* for future use */
)

STRUCT( sndrv_seq_instr_info,
	TYPE_INT,			/* operation result */
	MK_ARRAY(TYPE_INT, 8),	/* bitmap of supported formats */
	TYPE_INT,			/* count of RAM banks */
	MK_ARRAY(TYPE_INT, 16), /* size of RAM banks */
	TYPE_INT,			/* count of ROM banks */
	MK_ARRAY(TYPE_INT, 8), /* size of ROM banks */
	MK_ARRAY(TYPE_CHAR, 128)
)

STRUCT( sndrv_seq_instr_status,
	TYPE_INT,			/* operation result */
	MK_ARRAY(TYPE_INT, 16), /* free RAM in banks */
	TYPE_INT,		/* count of downloaded instruments */
	MK_ARRAY(TYPE_CHAR, 128)
)

STRUCT( sndrv_seq_instr_format_info,
	MK_ARRAY(TYPE_CHAR, 16),		/* format identifier - SNDRV_SEQ_INSTR_ID_* */
	TYPE_INT		/* max data length (without this structure) */
)

STRUCT( sndrv_seq_instr_format_info_result,
	TYPE_INT,			/* operation result */
	MK_ARRAY(TYPE_CHAR, 16),		/* format identifier */
	TYPE_INT		/* filled data length (without this structure) */
)

STRUCT( sndrv_seq_instr_data,
	MK_ARRAY(TYPE_CHAR, 32),			/* instrument name */
	MK_ARRAY(TYPE_CHAR, 16),		/* for the future use */
	TYPE_INT,			/* instrument type */
	MK_STRUCT(STRUCT_sndrv_seq_instr),
	MK_ARRAY(TYPE_CHAR, 4)	/* fillup */
)

STRUCT( sndrv_seq_instr_header,
	MK_STRUCT(STRUCT_sndrv_seq_instr),
	TYPE_INT,		/* get/put/free command */
	TYPE_INT,		/* query flags (only for get) */
	TYPE_INT,		/* real instrument data length (without header) */
	TYPE_INT,			/* operation result */
	MK_ARRAY(TYPE_CHAR, 16),		/* for the future */
	MK_STRUCT(STRUCT_sndrv_seq_instr_data) /* instrument data (for put/get result) */
)

STRUCT( sndrv_seq_instr_cluster_set,
	TYPE_INT, /* cluster identifier */
	MK_ARRAY(TYPE_CHAR, 32),			/* cluster name */
	TYPE_INT,			/* cluster priority */
	MK_ARRAY(TYPE_CHAR, 64)		/* for the future use */
)

STRUCT( sndrv_seq_instr_cluster_get,
	TYPE_INT, /* cluster identifier */
	MK_ARRAY(TYPE_CHAR, 32),			/* cluster name */
	TYPE_INT,			/* cluster priority */
	MK_ARRAY(TYPE_CHAR, 64)		/* for the future use */
)

STRUCT( snd_dm_fm_info,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( snd_dm_fm_voice,
	TYPE_CHAR,		/* operator cell (0 or 1) */
	TYPE_CHAR,		/* FM voice (0 to 17) */

	TYPE_CHAR,		/* amplitude modulation */
	TYPE_CHAR,		/* vibrato effect */
	TYPE_CHAR,	/* sustain phase */
	TYPE_CHAR,	/* keyboard scaling */
	TYPE_CHAR,		/* 4 bits: harmonic and multiplier */
	TYPE_CHAR,	/* 2 bits: decrease output freq rises */
	TYPE_CHAR,		/* 6 bits: volume */

	TYPE_CHAR,		/* 4 bits: attack rate */
	TYPE_CHAR,		/* 4 bits: decay rate */
	TYPE_CHAR,		/* 4 bits: sustain level */
	TYPE_CHAR,		/* 4 bits: release rate */

	TYPE_CHAR,		/* 3 bits: feedback for op0 */
	TYPE_CHAR,
	TYPE_CHAR,		/* stereo left */
	TYPE_CHAR,		/* stereo right */
	TYPE_CHAR		/* 3 bits: waveform shape */
)

STRUCT( snd_dm_fm_note,
	TYPE_CHAR,	/* 0-17 voice channel */
	TYPE_CHAR,	/* 3 bits: what octave to play */
	TYPE_INT,	/* 10 bits: frequency number */
	TYPE_CHAR
)

STRUCT( snd_dm_fm_params,
	TYPE_CHAR,		/* amplitude modulation depth (1=hi) */
	TYPE_CHAR,	/* vibrato depth (1=hi) */
	TYPE_CHAR,	/* keyboard split */
	TYPE_CHAR,		/* percussion mode select */

	/* This block is the percussion instrument data */
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( sndrv_aes_iec958,
	MK_ARRAY(TYPE_CHAR, 24),	/* AES/IEC958 channel status bits */
	MK_ARRAY(TYPE_CHAR, 147),	/* AES/IEC958 subcode bits */
	TYPE_CHAR,		/* nothing */
	MK_ARRAY(TYPE_CHAR, 4)	/* AES/IEC958 subframe bits */
)

STRUCT( sndrv_hwdep_info,
	TYPE_INT,		/* WR: device number */
	TYPE_INT,			/* R: card number */
	MK_ARRAY(TYPE_CHAR, 64),		/* ID (user selectable) */
	MK_ARRAY(TYPE_CHAR, 80),		/* hwdep name */
	TYPE_INT,			/* hwdep interface */
	MK_ARRAY(TYPE_CHAR, 64)	/* reserved for future */
)

/* generic DSP loader */
STRUCT( sndrv_hwdep_dsp_status,
	TYPE_INT,		/* R: driver-specific version */
	MK_ARRAY(TYPE_CHAR, 32),		/* R: driver-specific ID string */
	TYPE_INT,		/* R: number of DSP images to transfer */
	TYPE_INT,	/* R: bit flags indicating the loaded DSPs */
	TYPE_INT,	/* R: 1 = initialization finished */
	MK_ARRAY(TYPE_CHAR, 16)	/* reserved for future use */
)

STRUCT( sndrv_hwdep_dsp_image,
	TYPE_INT,		/* W: DSP index */
	MK_ARRAY(TYPE_CHAR, 64),		/* W: ID (e.g. file name) */
	TYPE_CHAR,		/* W: binary image */
	TYPE_LONG,			/* W: size of image in bytes */
	TYPE_LONG		/* W: driver-specific data */
)

STRUCT( sndrv_pcm_info,
	TYPE_INT,		/* RO/WR (control): device number */
	TYPE_INT,		/* RO/WR (control): subdevice number */
	TYPE_INT,			/* RO/WR (control): stream number */
	TYPE_INT,			/* R: card number */
	MK_ARRAY(TYPE_CHAR, 64),		/* ID (user selectable) */
	MK_ARRAY(TYPE_CHAR, 80),		/* name of this device */
	MK_ARRAY(TYPE_CHAR, 32),	/* subdevice name */
	TYPE_INT,			/* SNDRV_PCM_CLASS_* */
	TYPE_INT,		/* SNDRV_PCM_SUBCLASS_* */
	TYPE_INT,
	TYPE_INT,
	MK_ARRAY(TYPE_INT, 4),

	MK_ARRAY(TYPE_CHAR, 64)	/* reserved for future... */
)

STRUCT( sndrv_interval,
	TYPE_INT,
	TYPE_INT,
	TYPE_INTBITFIELD
)

STRUCT( sndrv_mask,
	MK_ARRAY(TYPE_INT, (SNDRV_MASK_MAX+31)/32)
)

STRUCT( sndrv_pcm_hw_params,
	TYPE_INT,
	MK_ARRAY(MK_STRUCT(STRUCT_sndrv_mask),SNDRV_PCM_HW_PARAM_LAST_MASK - SNDRV_PCM_HW_PARAM_FIRST_MASK + 1),
	MK_ARRAY(MK_STRUCT(STRUCT_sndrv_mask), 5),	/* reserved masks */
	MK_ARRAY(MK_STRUCT(STRUCT_sndrv_interval), SNDRV_PCM_HW_PARAM_LAST_INTERVAL - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL + 1),
	MK_ARRAY(MK_STRUCT(STRUCT_sndrv_interval), 9),	/* reserved intervals */
	TYPE_INT,		/* W: requested masks */
	TYPE_INT,		/* R: changed masks */
	TYPE_INT,		/* R: Info flags for returned setup */
	TYPE_INT,		/* R: used most significant bits */
	TYPE_INT,		/* R: rate numerator */
	TYPE_INT,		/* R: rate denominator */
	TYPE_LONG,	/* R: chip FIFO size in frames */
	MK_ARRAY(TYPE_CHAR, 64)	/* reserved for future */
)

STRUCT( sndrv_pcm_sw_params,
	TYPE_INT,			/* timestamp mode */
	TYPE_INT,
	TYPE_INT,			/* min ticks to sleep */
	TYPE_LONG,		/* min avail frames for wakeup */
	TYPE_LONG,		/* xfer size need to be a multiple */
	TYPE_LONG,	/* min hw_avail frames for automatic start */
	TYPE_LONG,	/* min avail frames for automatic stop */
	TYPE_LONG,	/* min distance from noise for silence filling */
	TYPE_LONG,	/* silence block size */
	TYPE_LONG,		/* poTYPE_INTers wrap point */
	MK_ARRAY(TYPE_CHAR, 64)		/* reserved for future */
)

STRUCT( sndrv_pcm_channel_info,
	TYPE_INT,
	TYPE_LONG,			/* mmap offset (FIXME) */
	TYPE_INT,		/* offset to first sample in bits */
	TYPE_INT		/* samples distance in bits */
)


STRUCT( sndrv_pcm_status,
	TYPE_INT,			/* stream state */
	MK_STRUCT(STRUCT_timespec),	/* time when stream was started/stopped/paused */
	MK_STRUCT(STRUCT_timespec),		/* reference timestamp */
	TYPE_LONG,	/* appl ptr */
	TYPE_LONG,	/* hw ptr */
	TYPE_LONG,	/* current delay in frames */
	TYPE_LONG,	/* number of frames available */
	TYPE_LONG,	/* max frames available on hw since last status */
	TYPE_LONG,	/* count of ADC (capture) overrange detections from last status */
	TYPE_INT,		/* suspended stream state */
	MK_ARRAY(TYPE_CHAR, 60)	/* must be filled with zero */
)

STRUCT( sndrv_pcm_mmap_status,
	TYPE_INT,			/* RO: state - SNDRV_PCM_STATE_XXXX */
	TYPE_INT,			/* Needed for 64 bit alignment */
	TYPE_LONG,	/* RO: hw ptr (0...boundary-1) */
	MK_STRUCT(STRUCT_timespec),		/* Timestamp */
	TYPE_INT		/* RO: suspended stream state */
)

STRUCT( sndrv_pcm_mmap_control,
	TYPE_LONG,	/* RW: appl ptr (0...boundary-1) */
	TYPE_LONG	/* RW: min available frames for wakeup */
)

STRUCT( sndrv_pcm_sync_ptr,
	TYPE_INT,
	// FIXME: does not work with 64-bit target
	MK_STRUCT(STRUCT_sndrv_pcm_mmap_status), // 28 bytes on 32-bit target
	MK_ARRAY(TYPE_CHAR, 64 - 24), // so we pad to 64 bytes (was a union)

	MK_STRUCT(STRUCT_sndrv_pcm_mmap_control), // 8 bytes on 32-bit target
	MK_ARRAY(TYPE_CHAR, 64 - 8) // so we pad to 64 bytes (was a union))
)

STRUCT( sndrv_xferi,
	TYPE_LONG,
	TYPE_PTRVOID,
	TYPE_LONG
)

STRUCT( sndrv_xfern,
	TYPE_LONG,
	TYPE_PTRVOID,
	TYPE_LONG
)

STRUCT( sndrv_rawmidi_info,
	TYPE_INT,		/* RO/WR (control): device number */
	TYPE_INT,		/* RO/WR (control): subdevice number */
	TYPE_INT,			/* WR: stream */
	TYPE_INT,			/* R: card number */
	TYPE_INT,		/* SNDRV_RAWMIDI_INFO_XXXX */
	MK_ARRAY(TYPE_CHAR, 64),		/* ID (user selectable) */
	MK_ARRAY(TYPE_CHAR, 80),		/* name of device */
	MK_ARRAY(TYPE_CHAR, 32),	/* name of active or selected subdevice */
	TYPE_INT,
	TYPE_INT,
	MK_ARRAY(TYPE_CHAR, 64)	/* reserved for future use */
)

STRUCT( sndrv_rawmidi_params,
	TYPE_INT,
	TYPE_LONG,		/* queue size in bytes */
	TYPE_LONG,		/* minimum avail bytes for wakeup */
	TYPE_INT, /* do not send active sensing byte in close() */
	MK_ARRAY(TYPE_CHAR, 16)	/* reserved for future use */
)

STRUCT( sndrv_rawmidi_status,
	TYPE_INT,
	MK_STRUCT(STRUCT_timespec),		/* Timestamp */
	TYPE_LONG,			/* available bytes */
	TYPE_LONG,			/* count of overruns since last status (in bytes) */
	MK_ARRAY(TYPE_CHAR, 16)	/* reserved for future use */
)

STRUCT( sndrv_timer_ginfo,
	MK_STRUCT(STRUCT_sndrv_timer_id),	/* requested timer ID */
	TYPE_INT,		/* timer flags - SNDRV_TIMER_FLG_* */
	TYPE_INT,			/* card number */
	MK_ARRAY(TYPE_CHAR, 64),		/* timer identification */
	MK_ARRAY(TYPE_CHAR, 80),		/* timer name */
	TYPE_LONG,	/* reserved for future use */
	TYPE_LONG,	/* average period resolution in ns */
	TYPE_LONG,	/* minimal period resolution in ns */
	TYPE_LONG,	/* maximal period resolution in ns */
	TYPE_INT,		/* active timer clients */
	MK_ARRAY(TYPE_CHAR, 32)
)

STRUCT( sndrv_timer_gparams,
	MK_STRUCT(STRUCT_sndrv_timer_id),	/* requested timer ID */
	TYPE_LONG,	/* requested precise period duration (in seconds) - numerator */
	TYPE_LONG,	/* requested precise period duration (in seconds) - denominator */
	MK_ARRAY(TYPE_CHAR, 32)
)

STRUCT( sndrv_timer_gstatus,
	MK_STRUCT(STRUCT_sndrv_timer_id),	/* requested timer ID */
	TYPE_LONG,	/* current period resolution in ns */
	TYPE_LONG,	/* precise current period resolution (in seconds) - numerator */
	TYPE_LONG,	/* precise current period resolution (in seconds) - denominator */
	MK_ARRAY(TYPE_CHAR, 32)
)

STRUCT( sndrv_timer_select,
	MK_STRUCT(STRUCT_sndrv_timer_id),	/* bind to timer ID */
	MK_ARRAY(TYPE_CHAR, 32)	/* reserved */
)

STRUCT( sndrv_timer_info,
	TYPE_INT,		/* timer flags - SNDRV_TIMER_FLG_* */
	TYPE_INT,			/* card number */
	MK_ARRAY(TYPE_CHAR, 64),		/* timer identificator */
	MK_ARRAY(TYPE_CHAR, 80),		/* timer name */
	TYPE_LONG,	/* reserved for future use */
	TYPE_LONG,	/* average period resolution in ns */
	MK_ARRAY(TYPE_CHAR, 64)	/* reserved */
)

STRUCT( sndrv_timer_params,
	TYPE_INT,		/* flags - SNDRV_MIXER_PSFLG_* */
	TYPE_INT,		/* requested resolution in ticks */
	TYPE_INT,	/* total size of queue (32-1024) */
	TYPE_INT,
	TYPE_INT,		/* event filter (bitmask of SNDRV_TIMER_EVENT_*) */
	MK_ARRAY(TYPE_CHAR, 60)	/* reserved */
)

STRUCT( sndrv_timer_status,
	MK_STRUCT(STRUCT_timespec),		/* Timestamp - last update */
	TYPE_INT,	/* current period resolution in ns */
	TYPE_INT,		/* counter of master tick lost */
	TYPE_INT,		/* count of read queue overruns */
	TYPE_INT,		/* used queue size */
	MK_ARRAY(TYPE_CHAR, 64)	/* reserved */
)

STRUCT( sndrv_timer_read,
	TYPE_INT,
	TYPE_INT
)

STRUCT( sndrv_timer_tread,
	TYPE_INT,
	MK_STRUCT(STRUCT_timespec),
	TYPE_INT
)

STRUCT( sndrv_ctl_card_info,
	TYPE_INT,			/* card number */
	TYPE_INT,			/* reserved for future (was type) */
	MK_ARRAY(TYPE_CHAR, 16),		/* ID of card (user selectable) */
	MK_ARRAY(TYPE_CHAR, 16),	/* Driver name */
	MK_ARRAY(TYPE_CHAR, 32),		/* Short name of soundcard */
	MK_ARRAY(TYPE_CHAR, 80),	/* name + info text about soundcard */
	MK_ARRAY(TYPE_CHAR, 16),	/* reserved for future (was ID of mixer) */
	MK_ARRAY(TYPE_CHAR, 80),	/* visual mixer identification */
	MK_ARRAY(TYPE_CHAR, 80),	/* card components / fine identification, delimited with one space (AC97 etc..) */
	MK_ARRAY(TYPE_CHAR, 48)	/* reserved for future */
)

STRUCT( sndrv_ctl_elem_id,
	TYPE_INT,
	TYPE_INT,			/* interface identifier */
	TYPE_INT,		/* device/client number */
	TYPE_INT,		/* subdevice (substream) number */
	MK_ARRAY(TYPE_CHAR, 44),		/* ASCII name of item */
	TYPE_INT		/* index of item */
)

STRUCT( sndrv_ctl_elem_list,
	TYPE_INT,		/* W: first element ID to get */
	TYPE_INT,		/* W: count of element IDs to get */
	TYPE_INT,		/* R: count of element IDs set */
	TYPE_INT,		/* R: count of all elements */
	MK_STRUCT(STRUCT_sndrv_ctl_elem_id), /* R: IDs */
	MK_ARRAY(TYPE_CHAR, 50)
)

STRUCT( sndrv_ctl_elem_info,
	MK_STRUCT(STRUCT_sndrv_ctl_elem_id),	/* W: element ID */
	TYPE_INT,			/* R: value type - SNDRV_CTL_ELEM_TYPE_* */
	TYPE_INT,		/* R: value access (bitmask) - SNDRV_CTL_ELEM_ACCESS_* */
	TYPE_INT,		/* count of values */
	TYPE_INT,			/* owner's PID of this control */
	MK_ARRAY(TYPE_CHAR, 128), // FIXME: prone to break (was union)
	MK_ARRAY(TYPE_SHORT, 4),		/* dimensions */
	MK_ARRAY(TYPE_CHAR, 64-4*sizeof(unsigned short))
)

STRUCT( sndrv_ctl_elem_value,
	MK_STRUCT(STRUCT_sndrv_ctl_elem_id),	/* W: element ID */
	TYPE_INT,	/* W: use indirect pointer (xxx_ptr member) */
	MK_ARRAY(TYPE_INT, 128),
	MK_STRUCT(STRUCT_timespec),
	MK_ARRAY(TYPE_CHAR, 128-sizeof(struct timespec)) // FIXME: breaks on 64-bit host
)

STRUCT( sndrv_ctl_tlv,
	TYPE_INT,     /* control element numeric identification */
        TYPE_INT,    /* in bytes aligned to 4 */
	MK_ARRAY(TYPE_INT, 0)    /* first TLV */ // FIXME: what is this supposed to become?
)

STRUCT( sndrv_ctl_event,
	TYPE_INT,				/* event type - SNDRV_CTL_EVENT_* */
	TYPE_INT,
	MK_STRUCT(STRUCT_sndrv_ctl_elem_id)    // 64 bytes
)

STRUCT( iovec,
	TYPE_PTRVOID,
	TYPE_LONG
      )


STRUCT( sndrv_xferv,
	MK_STRUCT(STRUCT_iovec),
	TYPE_LONG
)

STRUCT(emu10k1_fx8010_info,
	TYPE_INT,	/* in samples */
	TYPE_INT,	/* in samples */
	MK_ARRAY(MK_ARRAY(TYPE_CHAR, 32), 16),		/* names of FXBUSes */
	MK_ARRAY(MK_ARRAY(TYPE_CHAR, 32), 16),		/* names of external inputs */
	MK_ARRAY(MK_ARRAY(TYPE_CHAR, 32), 32),		/* names of external outputs */
	TYPE_INT		/* count of GPR controls */
)

STRUCT(emu10k1_ctl_elem_id,
	TYPE_INT,		/* don't use */
	TYPE_INT,			/* interface identifier */
	TYPE_INT,		/* device/client number */
	TYPE_INT,		/* subdevice (substream) number */
	MK_ARRAY(TYPE_CHAR, 44),		/* ASCII name of item */
	TYPE_INT		/* index of item */
)

STRUCT(emu10k1_fx8010_control_gpr,
	MK_STRUCT(STRUCT_emu10k1_ctl_elem_id),	/* full control ID definition */
	TYPE_INT,		/* visible count */
	TYPE_INT,		/* count of GPR (1..16) */
	MK_ARRAY(TYPE_SHORT, 32),		/* GPR number(s) */
	MK_ARRAY(TYPE_INT, 32),		/* initial values */
	TYPE_INT,		/* minimum range */
	TYPE_INT,		/* maximum range */
	TYPE_INT,	/* translation type (EMU10K1_GPR_TRANSLATION*) */
	TYPE_INT
)

#ifndef TARGET_LONG_SIZE
#define TARGET_LONG_SIZE 4
#endif

STRUCT(emu10k1_fx8010_code,
	MK_ARRAY(TYPE_CHAR, 128),

	MK_ARRAY(TYPE_LONG, 0x200/(TARGET_LONG_SIZE*8)), /* bitmask of valid initializers */
	TYPE_PTRVOID,		  /* initializers */

	TYPE_INT, /* count of GPR controls to add/replace */
	MK_STRUCT(STRUCT_emu10k1_fx8010_control_gpr), /* GPR controls to add/replace */

	TYPE_INT, /* count of GPR controls to remove */
	MK_STRUCT(STRUCT_emu10k1_ctl_elem_id), /* IDs of GPR controls to remove */

	TYPE_INT, /* count of GPR controls to list */
	TYPE_INT, /* total count of GPR controls */
	MK_STRUCT(STRUCT_emu10k1_fx8010_control_gpr), /* listed GPR controls */

	MK_ARRAY(TYPE_LONG, 0x100/(TARGET_LONG_SIZE*8)), /* bitmask of valid initializers */
	TYPE_PTRVOID,	/* data initializers */
	TYPE_PTRVOID,	/* map initializers */

	MK_ARRAY(TYPE_LONG, 1024/(TARGET_LONG_SIZE*8)),  /* bitmask of valid instructions */
	TYPE_PTRVOID	/* one instruction - 64 bits */
)

STRUCT(emu10k1_fx8010_tram,
	TYPE_INT,		/* 31.bit == 1 -> external TRAM */
	TYPE_INT,		/* size in samples (4 bytes) */
	TYPE_INT		/* pointer to samples (20-bit) */
					/* NULL->clear memory */
)

STRUCT(emu10k1_fx8010_pcm,
	TYPE_INT,		/* substream number */
	TYPE_INT,		/* reserved */
	TYPE_INT,
	TYPE_INT,	/* ring buffer position in TRAM (in samples) */
	TYPE_INT,	/* count of buffered samples */
	TYPE_SHORT,		/* GPR containing size of ringbuffer in samples (host) */
	TYPE_SHORT,
	TYPE_SHORT,	/* GPR containing count of samples between two TYPE_INTerrupts (host) */
	TYPE_SHORT,
	TYPE_SHORT,	/* GPR containing trigger (activate) information (host) */
	TYPE_SHORT,	/* GPR containing info if PCM is running (FX8010) */
	TYPE_CHAR,		/* reserved */
	MK_ARRAY(TYPE_CHAR, 32),	/* external TRAM address & data (one per channel) */
	TYPE_INT		/* reserved */
)

STRUCT( hdsp_peak_rms,
	MK_ARRAY(TYPE_INT, 26),
	MK_ARRAY(TYPE_INT, 26),
	MK_ARRAY(TYPE_INT, 28),
	MK_ARRAY(TYPE_LONGLONG, 26),
	MK_ARRAY(TYPE_LONGLONG, 26),
	/* These are only used for H96xx cards */
	MK_ARRAY(TYPE_LONGLONG, 26)
)

STRUCT( hdsp_config_info,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	MK_ARRAY(TYPE_CHAR, 3),
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_INT,
	TYPE_INT,
	TYPE_INT,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR,
	TYPE_CHAR
)

STRUCT( hdsp_firmware,
	TYPE_PTRVOID	/* 24413 x 4 bytes */
)

STRUCT( hdsp_version,
	TYPE_INT,
	TYPE_SHORT
)

STRUCT( hdsp_mixer,
	MK_ARRAY(TYPE_SHORT, HDSP_MATRIX_MIXER_SIZE)
)

STRUCT( hdsp_9632_aeb,
	TYPE_INT,
	TYPE_INT
)

STRUCT( snd_sb_csp_mc_header,
	MK_ARRAY(TYPE_CHAR, 16),		/* id name of codec */
	TYPE_SHORT	/* requested function */
)

STRUCT( snd_sb_csp_microcode,
	MK_STRUCT(STRUCT_snd_sb_csp_mc_header),
	MK_ARRAY(TYPE_CHAR, SNDRV_SB_CSP_MAX_MICROCODE_FILE_SIZE)
)

STRUCT( snd_sb_csp_start,
	TYPE_INT,
	TYPE_INT
)

STRUCT( snd_sb_csp_info,
	MK_ARRAY(TYPE_CHAR, 16),		/* id name of codec */
	TYPE_SHORT,		/* function number */
	TYPE_INT,	/* accepted PCM formats */
	TYPE_SHORT,	/* accepted channels */
	TYPE_SHORT,	/* accepted sample width */
	TYPE_SHORT,	/* accepted sample rates */
	TYPE_SHORT,
	TYPE_SHORT,	/* current channels  */
	TYPE_SHORT,	/* current sample width */
	TYPE_SHORT,		/* version id: 0x10 - 0x1f */
	TYPE_SHORT		/* state bits */
)

STRUCT( sscape_bootblock,
	MK_ARRAY(TYPE_CHAR, 256),
	TYPE_INT
)

STRUCT( sscape_microcode,
  TYPE_PTRVOID
)
