// Definições de Hardware
#define PIT_DATA_PORT_2 0x42      // Porta de dados do Canal 2 do PIT
#define PIT_CMD_PORT 0x43         // Porta de comando do PIT
#define SPEAKER_CONTROL_PORT 0x61 // Porta de controle do PC Speaker

// Funções de I/O (implementadas em Assembly e acessíveis via extern "C")
extern "C" void outb(unsigned short port, unsigned char data);
extern "C" unsigned char inb(unsigned short port);

// Funções de controle do PC Speaker
void speaker_play_frequency(unsigned int frequency_hz);
void speaker_stop();
void speaker_beep(unsigned int frequency_hz, unsigned int duration_ms); // Requer um timer