#pragma once
// communication protocol with emulator

// commands
#define INIT_DEVICE 1
#define GET_ASCAN 2
#define GET_STROBE 3

// responses
#define REPLY_ASCAN 2
#define REPLY_STROBE 3

#define NUM_RESULT_STRBS    5
#define NUM_USS             2
#define NUM_TACTS           4
#define NUM_CHANNELS        8

enum DetectionModeType
{
	Plus = 0,
	Minus,
	PlusMinus
};

struct strobeInfo
{
	quint8 value;
	float begin;
	float duration;
};

struct channelInfo
{
	quint8 channelNum;
	quint8 tact;
	quint8 ampl;
	float begin;
	float end;
	float amplF;
	float TT;
	float TV;
	DetectionModeType detMode;
	int numStrobes;
	strobeInfo strobes[NUM_RESULT_STRBS];
};

#pragma pack(push, 1)
struct amp_strob_struct_t
{
	quint16 time;
	quint8 ampl;
};

struct amp_us_struct_t
{
	amp_strob_struct_t ampl[NUM_RESULT_STRBS];
};

struct amp_tact_struct_t
{
	amp_us_struct_t ampl_us[NUM_USS];
};

struct amp_struct_t
{
	amp_tact_struct_t ampl_tact[NUM_TACTS];
};

struct channelData
{
	quint8 data[256];
	amp_strob_struct_t strobe[NUM_RESULT_STRBS];
};

#pragma pack(pop)
