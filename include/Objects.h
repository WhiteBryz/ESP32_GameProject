#ifndef NOSE_H
#define NOSE_H

#include <Arduino.h>

// Clase Global
class Objeto
{
public:
    int x, y;

    // Constructor
    Objeto(int x, int y)
    {
        this->x = x;
        this->y = y;
    }

    // Métodos
    int GetX(void);
    int GetY(void);
};

// Clase Diamante (hereda de Objeto)
class Diamante : public Objeto
{
public:
    // Constructor
    Diamante(int x, int y) : Objeto(x, y) {}

    // Métodos
    void RehubicarObjeto(void);
    bool Colision(int x1, int y1, int x2, int y2);
};

// Clase Jugador (hereda de Objeto)
class Personaje : public Objeto
{
public:
    int puntaje;
    char *nombre;

    // Constructor
    Personaje(int x, int y, int puntaje = 0, char *nombre = nullptr) : Objeto(x, y)
    {
        this->puntaje = puntaje;
        this->nombre = nombre;
    }

    // Métodos
    void Left(void);
    void Right(void);
    void Up(void);
    void Down(void);
    void IncrementarPuntaje(void);
    int ImprimirPuntaje(void);
    void ReiniciarValores(void);
    void AsignarNombre(char *nombre);
    char *ImprimirNombre(void);
};

// Desarrollo de métodos

// Métodos Objeto
int Objeto::GetX(void)
{
    return x;
}

int Objeto::GetY(void)
{
    return y;
}

// Métodos Diamante
void Diamante::RehubicarObjeto(void)
{
    x = random(15); // Suponiendo que estás usando una pantalla de 16 columnas
    y = random(2);  // Suponiendo que hay 2 filas
}

bool Diamante::Colision(int x1, int y1, int x2, int y2)
{
    return (x1 == x2 && y1 == y2);
}

// Métodos Personaje
void Personaje::Left(void)
{
    if (x > 0)
    {
        x -= 1;
    }
}

void Personaje::Right(void)
{
    if (x < 15)
    { // Suponiendo que el límite es 15
        x += 1;
    }
}

void Personaje::Up(void)
{
    y = 0;
}

void Personaje::Down(void)
{
    y = 1;
}

void Personaje::IncrementarPuntaje(void)
{
    puntaje++;
}

int Personaje::ImprimirPuntaje(void)
{
    return puntaje;
}

void Personaje::ReiniciarValores(void)
{
    puntaje = 0;
    nombre = nullptr;
    x = 0;
    y = 0;
}

void Personaje::AsignarNombre(char *nombre)
{
    nombre = nombre;
}
char *Personaje::ImprimirNombre(void)
{
    return nombre;
}

#endif
