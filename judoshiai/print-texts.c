/* -*- mode: C; c-basic-offset: 4;  -*- */

/*
 * Copyright (C) 2006-2011 by Hannu Jokinen
 * Full copyright text is included in the software package.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "judoshiai.h"

gchar *print_texts[numprinttexts][NUM_LANGS] = {
    /* fi               se            en               es               et               uk  */
    {"Kilpailijat", "Tävlande", "Competitors", "Competidores", "", "Учасники"},
    {"Kilpailijat A", "Tävlande A", "Competitors A", "Competidores A", "", "Учасники A"},
    {"Kilpailijat B", "Tävlande B", "Competitors B", "Competidores B", "", "Учасники B"},
    {"Kilpailijat C", "Tävlande C", "Competitors C", "Competidores C", "", "Учасники C"},
    {"Kilpailijat D", "Tävlande D", "Competitors D", "Competidores D", "", "Учасники D"},
    {"N:o", "Nr", "#", "Nº", "", "№"},
    {"Nimi", "Namn", "Name", "Nombre", "", "Учасник"},
    {"Vyö", "Grad", "Grade", "Kyu", "", "Розряд"},
    {"Seura", "Klubb", "Club", "Club", "", "Команда"},
    {"Ottelut", "Matcherna", "Matches", "Combates", "", "Сутички"},
    {"Ottelut A", "Matcherna A", "Matches A", "Combates A", "", "Сутички A"},
    {"Ottelut B", "Matcherna B", "Matches B", "Combates B", "", "Сутички B"},
    {"Ottelut C", "Matcherna C", "Matches C", "Combates C", "", "Сутички C"},
    {"Ottelut D", "Matcherna D", "Matches D", "Combates D", "", "Сутички D"},

    {"Ottelu", "Match", "Match", "Comb.", "", "Сутичка"},
    {"Sininen", "Blå", "Blue", "Azul", "", "Синій"},
    {"Valkoinen", "Vit", "White", "Blanco", "", "Білий"},
    {"Tulos", "Resultat", "Result", "Resultado", "", "Результат"},
    {"Tulokset", "Resultat", "Results", "Clasificación", "", "Результати"},
    {"Aika", "Tid", "Time", "Tiempo", "", "Час"},
    {"Voit.", "Vin.", "Wins", "Victs.", "", "П"},
    {"Pist.", "Poäng", "Pts", "Pts.", "", "О"},
    {"Sij.", "Plats", "Pos", "Pos.", "", "М"},
    {"Ed. voit", "Föregående vinnare", "Previous winner", "Ganador anterior", "", "Previous winner"},

    {"Valmistautuvat", "Därefter", "Next", "Siguiente", "", "Наступні"},
    {"Paino:", "Vikt:", "Weight:", "Peso:", "", "Вага"},
    {"ALUSTAVA AIKATAULU", "PRELIMINÄR TIDTABELL", "PRELIMINARY SCHEDULE", "AGENDA PRELIMINAR", "", "ПОПЕРЕДНІЙ РОЗКЛАД"},
    {"punnituslaput.pdf", "viktlappor.pdf", "weighin-notes.pdf", "notas-pesaje.pdf", "weighin-notes.pdf", "weighin-notes.pdf"},
    {"aikataulu.pdf", "tidtabell.pdf", "schedule.pdf", "agenda.pdf", "schedule.pdf", "schedule.pdf"},
    {"sarjat.pdf", "viktklass.pdf", "categories.pdf", "categorias.pdf", "categories.pdf", "categories.pdf"},
    {"Sarja", "Viktklass", "Category", "Categoria", "", "Категорія"},
    {"Seuraavat ottelut", "Nästa match", "Next fights", "Siguiente combate", "", "Наступна"},
    {"Sarjat", "Viktklass", "Categories", "Categorias", "", "Категорії"},
    {"Tilastot", "Statistik", "Statistics", "Estadísticas", "", "Статистика"},

    {"Mitalit", "Medaljer", "Medals", "Medallas", "", "Медалі"},
    {"Yhteensä", "Totalt", "Total", "Total", "", "Підсумок"}
};
