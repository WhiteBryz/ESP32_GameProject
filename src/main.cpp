#include "DualCore.h"

// Creación de objeto para habilitar tareas en DualCore
DualCoreESP32 DCESP32;

void setup(void)
{
  Serial.begin(115200);

  // --- INICIALIZACIÓN DEL DUALCORE ---
  DCESP32.ConfigCores();
  Serial.println(F("Se han configurado correctamente los dos nucleos"));
  // Pasamos la primera bandera al Intro del juego
  ChangeGameState(STATE_MENU);
}

void loop(void)
{
}
