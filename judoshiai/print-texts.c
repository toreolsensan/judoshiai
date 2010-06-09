/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2010 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "judoshiai.h"

gchar *print_texts[numprinttexts][NUM_LANGS] = {
    {"Kilpailijat", "Tävlande", "Competitors", "Competidores"},
    {"Kilpailijat A", "Tävlande A", "Competitors A", "Competidores A"},
    {"Kilpailijat B", "Tävlande B", "Competitors B", "Competidores B"},
    {"N:o", "Nr", "#", "Nº"},
    {"Nimi", "Namn", "Name", "Nombre"},
    {"Vyö", "Grad", "Grade", "Kyu"},
    {"Seura", "Klubb", "Club", "Club"},
    {"Ottelut", "Matcherna", "Matches", "Combates"},
    {"Ottelut A", "Matcherna A", "Matches A", "Combates A"},
    {"Ottelut B", "Matcherna B", "Matches B", "Combates "},

    {"Ottelu", "Match", "Match", "Combate"},
    {"Sininen", "Blå", "Blue", "Azul"},
    {"Valkoinen", "Vit", "White", "Blanco"},
    {"Tulos", "Resultat", "Result", "Resultado"},
    {"Tulokset", "Resultat", "Results", "Resultados"},
    {"Aika", "Tid", "Time", "Tiempo"},
    {"Voit.", "Vin.", "Wins", "Victorias"},
    {"Pist.", "Poäng", "Pts", "Pts."},
    {"Sij.", "Plats", "Pos", "Pos."},
    {"Ed. voit", "Föregående vinnare", "Previous winner", "Ganador anterior"},

    {"Valmistautuvat", "Därefter", "Next", "Siguiente"},
    {"Paino:", "Vikt:", "Weight:", "Peso:"},
    {"ALUSTAVA AIKATAULU", "PRELIMINÄR TIDTABELL", "PRELIMINARY SCHEDULE", "AGENDA PRELIMINAR"},
    {"punnituslaput.pdf", "viktlappor.pdf", "weighin-notes.pdf", "notas-pesaje.pdf"},
    {"aikataulu.pdf", "tidtabell.pdf", "schedule.pdf", "agenda.pdf"},
    {"sarjat.pdf", "viktklass.pdf", "categories.pdf", "categorias.pdf"},
    {"Sarja", "Viktklass", "Category", "Categoria"},
    {"Seuraavat ottelut", "Nästa match", "Next fights", "Siguiente combate"},
    {"Sarjat", "Viktklass", "Categories", "Categorias"},
    {"Tilastot", "Statistik", "Statistics", "Estadísticas"},

    {"Mitalit", "Medaljer", "Medals", "Medallas"},
    {"Yhteensä", "Totalt", "Total", "Total"}
};
