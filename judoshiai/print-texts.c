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
    /* fi               se            en               es               et               uk               is               no  */
    {"Kilpailijat", "Tävlande", "Competitors", "Competidores", "", "Учасники", "Keppendur", "Deltakere"},
    {"Kilpailijat A", "Tävlande A", "Competitors A", "Competidores A", "", "Учасники A", "Keppendur A", "Deltakere A"},
    {"Kilpailijat B", "Tävlande B", "Competitors B", "Competidores B", "", "Учасники B", "Keppendur B", "Deltakere B"},
    {"Kilpailijat C", "Tävlande C", "Competitors C", "Competidores C", "", "Учасники C", "Keppendur C", "Deltakere C"},
    {"Kilpailijat D", "Tävlande D", "Competitors D", "Competidores D", "", "Учасники D", "Keppendur D", "Deltakere D"},
    {"N:o", "Nr", "#", "Nº", "", "№", "Nr", "Nr"},
    {"Nimi", "Namn", "Name", "Nombre", "", "Учасник", "Nafn", "Navn"},
    {"Vyö", "Grad", "Grade", "Kyu", "", "Розряд", "Gráða", "Grad"},
    {"Seura", "Klubb", "Club", "Club", "", "Команда", "Félag", "Klubb"},
    {"Ottelut", "Matcherna", "Matches", "Combates", "", "Сутички", "Viðureignir", "Kamper"},
    {"Ottelut A", "Matcherna A", "Matches A", "Combates A", "", "Сутички A", "Viðureignir A", "Kamper A"},
    {"Ottelut B", "Matcherna B", "Matches B", "Combates B", "", "Сутички B", "Viðureignir B", "Kamper B"},
    {"Ottelut C", "Matcherna C", "Matches C", "Combates C", "", "Сутички C", "Viðureignir C", "Kamper C"},
    {"Ottelut D", "Matcherna D", "Matches D", "Combates D", "", "Сутички D", "Viðureignir D", "Kamper D"},

    {"Ottelu", "Match", "Match", "Comb.", "", "Сутичка", "Viðureign", "Kamp"},
    {"Sininen", "Blå", "Blue", "Azul", "", "Синій", "Blár", "Blå"},
    {"Valkoinen", "Vit", "White", "Blanco", "", "Білий", "Hvítur", "Hvit"},
    {"Tulos", "Resultat", "Result", "Resultado", "", "Результат", "Úrslit", "Resultat"},
    {"Tulokset", "Resultat", "Results", "Clasificación", "", "Результати", "Úrslit", "Resultater"},
    {"Aika", "Tid", "Time", "Tiempo", "", "Час", "Tími", "Tid"},
    {"Voit.", "Vin.", "Wins", "Victs.", "", "П", "Vin.", "Seire"},
    {"Pist.", "Poäng", "Pts", "Pts.", "", "О", "Stig", "Poeng"},
    {"Sij.", "Plats", "Pos", "Pos.", "", "М", "Sæti", "Plass"},
    {"Ed. voit", "Föregående vinnare", "Previous winner", "Ganador anterior", "", "Previous winner", "Fyrri sigurvegari", "Forrige Vinner"},

    {"Valmistautuvat", "Därefter", "Next", "Siguiente", "", "Наступні", "Næsta", "Neste"},
    {"Paino:", "Vikt:", "Weight:", "Peso:", "", "Вага", "Þyngd", "Vekt"},
    {"ALUSTAVA AIKATAULU", "PRELIMINÄR TIDTABELL", "PRELIMINARY SCHEDULE", "AGENDA PRELIMINAR", "", "ПОПЕРЕДНІЙ РОЗКЛАД", "Bráðabirgða áætlun", "FORELØPIG TIDSSKJEMA"},
    {"punnituslaput.pdf", "viktlappor.pdf", "weighin-notes.pdf", "notas-pesaje.pdf", "weighin-notes.pdf", "weighin-notes.pdf","vigtunnar-listi", "veielapper.pdf"},
    {"aikataulu.pdf", "tidtabell.pdf", "schedule.pdf", "agenda.pdf", "schedule.pdf", "schedule.pdf", "áætlun", "tidsskjema.pdf"},
    {"sarjat.pdf", "viktklass.pdf", "categories.pdf", "categorias.pdf", "categories.pdf", "categories.pdf", "flokkar", "vektklasser.pdf"},
    {"Sarja", "Viktklass", "Category", "Categoria", "", "Категорія", "Þyngdarflokkur", "Vektklasse"},
    {"Seuraavat ottelut", "Nästa match", "Next fights", "Siguiente combate", "", "Наступна", "Næstu viðureignir", "Neste kamp"},
    {"Sarjat", "Viktklass", "Categories", "Categorias", "", "Категорії", "Þyngdarflokkar", "Vektklasser"},
    {"Tilastot", "Statistik", "Statistics", "Estadísticas", "", "Статистика", "Tölfræði", "Statistikk"},

    {"Mitalit", "Medaljer", "Medals", "Medallas", "", "Медалі", "Verðlaun", "Medaljer"},
    {"Yhteensä", "Totalt", "Total", "Total", "", "Підсумок", "Samtals", "Total"},
    {"Maa", "Land", "Country", "País", "Riik", "Країна", "Land", "Land"}
};
