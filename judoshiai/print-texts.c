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
    /* fi               se            en               es               et               uk               is               no               pl*/
    {"Kilpailijat", "Tävlande", "Competitors", "Competidores", "", "Учасники", "Keppendur", "Deltakere", "Zawodnicy"},
    {"Kilpailijat A", "Tävlande A", "Competitors A", "Competidores A", "", "Учасники A", "Keppendur A", "Deltakere A", "Zawodnicy A"},
    {"Kilpailijat B", "Tävlande B", "Competitors B", "Competidores B", "", "Учасники B", "Keppendur B", "Deltakere B", "Zawodnicy B"},
    {"Kilpailijat C", "Tävlande C", "Competitors C", "Competidores C", "", "Учасники C", "Keppendur C", "Deltakere C", "Zawodnicy C"},
    {"Kilpailijat D", "Tävlande D", "Competitors D", "Competidores D", "", "Учасники D", "Keppendur D", "Deltakere D", "Zawodnicy D"},
    {"N:o", "Nr", "#", "Nº", "", "№", "Nr", "Nr", "Nr"},
    {"Nimi", "Namn", "Name", "Nombre", "", "Учасник", "Nafn", "Navn", "Imię"},
    {"Vyö", "Grad", "Grade", "Kyu", "", "Розряд", "Gráða", "Grad", "Stopień"},
    {"Seura", "Klubb", "Club", "Club", "", "Команда", "Félag", "Klubb", "Klub"},
    {"Ottelut", "Matcherna", "Matches", "Combates", "", "Сутички", "Viðureignir", "Kamper", "Walki"},
    {"Ottelut A", "Matcherna A", "Matches A", "Combates A", "", "Сутички A", "Viðureignir A", "Kamper A", "Walki A"},
    {"Ottelut B", "Matcherna B", "Matches B", "Combates B", "", "Сутички B", "Viðureignir B", "Kamper B", "Walki B"},
    {"Ottelut C", "Matcherna C", "Matches C", "Combates C", "", "Сутички C", "Viðureignir C", "Kamper C", "Walki C"},
    {"Ottelut D", "Matcherna D", "Matches D", "Combates D", "", "Сутички D", "Viðureignir D", "Kamper D", "Walki D"},

    {"Ottelu", "Match", "Match", "Comb.", "", "Сутичка", "Viðureign", "Kamp", "Walka"},
    {"Sininen", "Blå", "Blue", "Azul", "", "Синій", "Blár", "Blå", "Niebieski"},
    {"Valkoinen", "Vit", "White", "Blanco", "", "Білий", "Hvítur", "Hvit", "Biały"},
    {"Tulos", "Resultat", "Result", "Resultado", "", "Результат", "Úrslit", "Resultat", "Wynik"},
    {"Tulokset", "Resultat", "Results", "Clasificación", "", "Результати", "Úrslit", "Resultater", "Wyniki"},
    {"Aika", "Tid", "Time", "Tiempo", "", "Час", "Tími", "Tid", "Czas"},
    {"Voit.", "Vin.", "Wins", "Victs.", "", "П", "Vin.", "Seire", "Wygrywa"},
    {"Pist.", "Poäng", "Pts", "Pts.", "", "О", "Stig", "Poeng", "Pkt."},
    {"Sij.", "Plats", "Pos", "Pos.", "", "М", "Sæti", "Plass", "Miejsce"},
    {"Ed. voit", "Föregående vinnare", "Previous winner", "Ganador anterior", "", "Previous winner", "Fyrri sigurvegari", "Forrige Vinner", "Poprzedni wygrany"},

    {"Valmistautuvat", "Därefter", "Next", "Siguiente", "", "Наступні", "Næsta", "Neste", "Następny"},
    {"Paino:", "Vikt:", "Weight:", "Peso:", "", "Вага", "Þyngd", "Vekt", "Waga:"},
    {"ALUSTAVA AIKATAULU", "PRELIMINÄR TIDTABELL", "PRELIMINARY SCHEDULE", "AGENDA PRELIMINAR", "", "ПОПЕРЕДНІЙ РОЗКЛАД", "Bráðabirgða áætlun", "FORELØPIG TIDSSKJEMA", "WSTĘPNY PLAN"},
    {"punnituslaput.pdf", "viktlappor.pdf", "weighin-notes.pdf", "notas-pesaje.pdf", "weighin-notes.pdf", "weighin-notes.pdf","vigtunnar-listi", "veielapper.pdf", "waga-notatki.pdf"},
    {"aikataulu.pdf", "tidtabell.pdf", "schedule.pdf", "agenda.pdf", "schedule.pdf", "schedule.pdf", "áætlun", "tidsskjema.pdf", "plan.pdf"},
    {"sarjat.pdf", "viktklass.pdf", "categories.pdf", "categorias.pdf", "categories.pdf", "categories.pdf", "flokkar", "vektklasser.pdf", "kategorie.pdf"},
    {"Sarja", "Viktklass", "Category", "Categoria", "", "Категорія", "Þyngdarflokkur", "Vektklasse", "Kategoria"},
    {"Seuraavat ottelut", "Nästa match", "Next fights", "Siguiente combate", "", "Наступна", "Næstu viðureignir", "Neste kamp", "Kolejne walki"},
    {"Sarjat", "Viktklass", "Categories", "Categorias", "", "Категорії", "Þyngdarflokkar", "Vektklasser", "Kategorie"},
    {"Tilastot", "Statistik", "Statistics", "Estadísticas", "", "Статистика", "Tölfræði", "Statistikk", "Statystyki"},

    {"Mitalit", "Medaljer", "Medals", "Medallas", "", "Медалі", "Verðlaun", "Medaljer", "Medale"},
    {"Yhteensä", "Totalt", "Total", "Total", "", "Підсумок", "Samtals", "Total", "Suma"},
    {"Maa", "Land", "Country", "País", "Riik", "Країна", "Land", "Land", "Kraj"},

    // Coach display texts
    {"Ei arvottu", "Not drawn", "Not drawn", "Not drawn", "Not drawn", "Not drawn", "Not drawn", "Not drawn", "Not drawn"},
    {"Loppunut", "Finished", "Finished", "Finished", "Finished", "Finished", "Finished", "Finished", "Finished"},
    {"Ottelu käynnissä", "Match ongoing", "Match ongoing", "Match ongoing", "Match ongoing", "Match ongoing", "Match ongoing", "Match ongoing", "Match ongoing"},
    {"Sarja alkanut", "Started", "Started", "Started", "Started", "Started", "Started", "Started", "Started"},
    {"Arvonta valmis", "Drawing ready", "Drawing ready", "Drawing ready", "Drawing ready", "Drawing ready", "Drawing ready", "Drawing ready", "Drawing ready"},
    {"Valmentaja", "Coach", "Coach", "Coach", "Coach", "Coach", "Coach", "Coach", "Coach"},
    {"Sukunimi", "Surname", "Surname", "Surname", "Surname", "Surname", "Surname", "Surname", "Surname"},
    {"Tilanne", "Status", "Status", "Status", "Status", "Status", "Status", "Status", "Status"},
    {"näyttö", "Display", "Display", "Display", "Display", "Display", "Display", "Display", "Display"},
    // Text: Match after n matches
    {"Ottelu", "Match after", "Match after", "Match after", "Match after", "Match after", "Match after", "Match after", "Match after"},
    {"ottelun jälkeen", "matches", "matches", "matches", "matches", "matches", "matches", "matches", "matches"},
};
