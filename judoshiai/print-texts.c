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

gchar *print_texts[numprinttexts][3] = {
    {"Kilpailijat", "Tävlande", "Competitors"},
    {"Kilpailijat A", "Tävlande A", "Competitors A"},
    {"Kilpailijat B", "Tävlande B", "Competitors B"},
    {"N:o", "Nr", "#"},
    {"Nimi", "Namn", "Name"},
    {"Vyö", "Grad", "Grade"},
    {"Seura", "Klubb", "Club"},
    {"Ottelut", "Matcherna", "Matches"},
    {"Ottelut A", "Matcherna A", "Matches A"},
    {"Ottelut B", "Matcherna B", "Matches B"},

    {"Ottelu", "Match", "Match"},
    {"Sininen", "Blå", "Blue"},
    {"Valkoinen", "Vit", "White"},
    {"Tulos", "Resultat", "Result"},
    {"Tulokset", "Resultat", "Results"},
    {"Aika", "Tid", "Time"},
    {"Voit.", "Vin.", "Wins"},
    {"Pist.", "Poäng", "Pts"},
    {"Sij.", "Plats", "Pos"},
    {"Ed. voit", "Föregående vinnare", "Previous winner"},

    {"Valmistautuvat", "Därefter", "Next"},
    {"Paino:", "Vikt:", "Weight:"},
    {"ALUSTAVA AIKATAULU", "PRELIMINÄR TIDTABELL", "PRELIMINARY SCHEDULE"},
    {"punnituslaput.pdf", "viktlappor.pdf", "weighin-notes.pdf"},
    {"aikataulu.pdf", "tidtabell.pdf", "schedule.pdf"},
    {"sarjat.pdf", "viktklass.pdf", "categories.pdf"},
    {"Sarja", "Viktklass", "Category"},
    {"Seuraavat ottelut", "Nästa match", "Next fights"},
    {"Sarjat", "Viktklass", "Categories"},
    {"Tilastot", "Statistik", "Statistics"},

    {"Mitalit", "Medaljer", "Medals"},
    {"Yhteensä", "Totalt", "Total"}
};
