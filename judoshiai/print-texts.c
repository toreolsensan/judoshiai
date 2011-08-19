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
    /* fi               se            en               es               et               uk               is  */
    {"Kilpailijat", "Tävlande", "Competitors", "Competidores", "", "Учасники", "Keppendur"},
    {"Kilpailijat A", "Tävlande A", "Competitors A", "Competidores A", "", "Учасники A", "Keppendur A"},
    {"Kilpailijat B", "Tävlande B", "Competitors B", "Competidores B", "", "Учасники B", "Keppendur B"},
    {"Kilpailijat C", "Tävlande C", "Competitors C", "Competidores C", "", "Учасники C", "Keppendur C"},
    {"Kilpailijat D", "Tävlande D", "Competitors D", "Competidores D", "", "Учасники D", "Keppendur D"},
    {"N:o", "Nr", "#", "Nº", "", "№", "Nr"},
    {"Nimi", "Namn", "Name", "Nombre", "", "Учасник", "Nafn"},
    {"Vyö", "Grad", "Grade", "Kyu", "", "Розряд", "Gráða"},
    {"Seura", "Klubb", "Club", "Club", "", "Команда", "Félag"},
    {"Ottelut", "Matcherna", "Matches", "Combates", "", "Сутички", "Viðureignir"},
    {"Ottelut A", "Matcherna A", "Matches A", "Combates A", "", "Сутички A", "Viðureignir A"},
    {"Ottelut B", "Matcherna B", "Matches B", "Combates B", "", "Сутички B", "Viðureignir B"},
    {"Ottelut C", "Matcherna C", "Matches C", "Combates C", "", "Сутички C", "Viðureignir C"},
    {"Ottelut D", "Matcherna D", "Matches D", "Combates D", "", "Сутички D", "Viðureignir D"},

    {"Ottelu", "Match", "Match", "Comb.", "", "Сутичка", "Viðureign"},
    {"Sininen", "Blå", "Blue", "Azul", "", "Синій", "Blár"},
    {"Valkoinen", "Vit", "White", "Blanco", "", "Білий", "Hvítur"},
    {"Tulos", "Resultat", "Result", "Resultado", "", "Результат", "Úrslit"},
    {"Tulokset", "Resultat", "Results", "Clasificación", "", "Результати", "Úrslit"},
    {"Aika", "Tid", "Time", "Tiempo", "", "Час", "Tími"},
    {"Voit.", "Vin.", "Wins", "Victs.", "", "П", "Vinningar"},
    {"Pist.", "Poäng", "Pts", "Pts.", "", "О", "Stig"},
    {"Sij.", "Plats", "Pos", "Pos.", "", "М", "Sæti"},
    {"Ed. voit", "Föregående vinnare", "Previous winner", "Ganador anterior", "", "Previous winner", "Fyrri sigurvegari"},

    {"Valmistautuvat", "Därefter", "Next", "Siguiente", "", "Наступні", "Næsta"},
    {"Paino:", "Vikt:", "Weight:", "Peso:", "", "Вага", "Þyngd"},
    {"ALUSTAVA AIKATAULU", "PRELIMINÄR TIDTABELL", "PRELIMINARY SCHEDULE", "AGENDA PRELIMINAR", "", "ПОПЕРЕДНІЙ РОЗКЛАД", "Bráðabirgða áætlun"},
    {"punnituslaput.pdf", "viktlappor.pdf", "weighin-notes.pdf", "notas-pesaje.pdf", "weighin-notes.pdf", "weighin-notes.pdf","vigtunnar-listi"},
    {"aikataulu.pdf", "tidtabell.pdf", "schedule.pdf", "agenda.pdf", "schedule.pdf", "schedule.pdf", "áætlun"},
    {"sarjat.pdf", "viktklass.pdf", "categories.pdf", "categorias.pdf", "categories.pdf", "categories.pdf", "flokkar"},
    {"Sarja", "Viktklass", "Category", "Categoria", "", "Категорія", "Þyngdarflokkur"},
    {"Seuraavat ottelut", "Nästa match", "Next fights", "Siguiente combate", "", "Наступна", "Næstu viðureignir"},
    {"Sarjat", "Viktklass", "Categories", "Categorias", "", "Категорії", "Þyngdarflokkar"},
    {"Tilastot", "Statistik", "Statistics", "Estadísticas", "", "Статистика", "Tölfræði"},

    {"Mitalit", "Medaljer", "Medals", "Medallas", "", "Медалі", "Verðlaun"},
    {"Yhteensä", "Totalt", "Total", "Total", "", "Підсумок", "Samtals"}
};
