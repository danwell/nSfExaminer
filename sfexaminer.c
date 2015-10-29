/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#define CLASS_HERO 16
#define CLASS_WIZD 20
#define CLASS_VICR 22
#define CLASS_JGRT 30

bool readChar(FILE* file, int* value) {
        int readChar = fgetc(file);
        if(readChar == EOF)
                return false;

        *value = readChar;
        return true;
}

bool readInt16BE(FILE* file, int* value) {
        int r1 = fgetc(file);
        if(r1 == EOF)
                return false;

        int r2 = fgetc(file);
        if(r2 == EOF)
                return false;

        *value = (r1 << 8) + r2;
        return true;
}

/*
 * Party Member Data specification in save state
 *
 * I suspect that `name' is actually 10 chars, but I need to check this
 * Intresting note; battle GUI supports 10 characters and terminates
 * text correctly. Outside battle GUI glitches out as it doesn't assume
 * a null terminator after 8 characters
 *
 * If the full 10 characters are used, there is no terminating character
 * in the name. PartyMember will allocate 11 characters for the name
 * with the last character being `\0' to work around this.
 *
 * Using the offsets of `Khris' as an example:
 *     C362: Name
 *     C363: Name
 *     C364: Name
 *     C365: Name
 *     C366: Name
 *     C367: Name
 *     C368: Name
 *     C369: Name
 *     C36A: Name? (Must be 0x00 for outside-battle GUI to not glitch)
 *     C36B: Name? (In-battle GUI assumes null for char 11, so this display correctly)
 *     C36C: Class
 *     C36D: Level
 *     C36E: Attack
 *     C36F: Defence
 *     C370: Agility
 *     C371: Move
 *     C372: (Possibly Crit?)
 *     C373: Experience
 *     C374: Current HP
 *     C375: Current HP
 *     C376: Maximum HP
 *     C377: Maximum HP
 *     C378: Current MP
 *     C379: Maximum MP
 *     C37A: ? (Always 0x00?)
 *     C37B: ? (Always 0x00?)
 *     C37C: Item 1 (Top)
 *     C37D: Item 2 (Left?)
 *     C37E: Item 3 (Right?)
 *     C37F: Item 4 (Bottom)
 *     C380: Magic 1
 *     C381: Magic 2
 *     C382: Magic 3
 *     C383: Magic 4
 *     C384: ? (Always 0x00?)
 *     C385: ? (Always 0x00?)
 *     C386: ? (Always 0x00?)
 *     C387: ? (Always 0x00?)
 *     C388: ? (Always 0x00?)
 *     C389: ? (Always 0x00?)
 */

struct partymember {
        char* name;
        unsigned char class;
        unsigned char level;
        unsigned char attack;
        unsigned char defence;
        unsigned char agility;
        unsigned char movement;
        unsigned char critical;
        unsigned char experience;
        int currentHP;
        int maximumHP;
        unsigned char currentMP;
        unsigned char maximumMP;
        char* padding1;
        unsigned char* items;
        unsigned char* magic;
        char* padding2;
};

struct partymember* CreatePartyMember() {
        struct partymember* pm = malloc(sizeof(*pm));
        pm->name = (char*) malloc(sizeof(char) * 11);
        pm->name[10] = '\0';
        pm->padding1 = (char*) malloc(sizeof(char) * 2);
        pm->items = (unsigned char*) malloc(sizeof(unsigned char) * 4);
        pm->magic = (unsigned char*) malloc(sizeof(unsigned char) * 4);
        pm->padding2 = (char*) malloc(sizeof(char) * 6);
}

void FreePartyMember(struct partymember* pm) {
        free(pm->name);
        free(pm->padding1);
        free(pm->items);
        free(pm->magic);
        free(pm->padding2);
}


#define LPMREADBYTE if(!readChar(file, &readchr)) return false;
bool LoadPartyMember(FILE* file, struct partymember* pm) {
        int readchr;

        for(int i = 0; i < 10; i++) {
                readchr = fgetc(file);
                if(readchr == -1)
                        return false;
                pm->name[i] = readchr;
        }

        LPMREADBYTE;
        pm->class = readchr;
        LPMREADBYTE;
        pm->level = readchr;
        LPMREADBYTE;
        pm->attack = readchr;
        LPMREADBYTE;
        pm->defence = readchr;
        LPMREADBYTE;
        pm->agility = readchr;
        LPMREADBYTE;
        pm->movement = readchr;
        LPMREADBYTE;
        pm->critical = readchr;
        LPMREADBYTE;
        pm->experience = readchr;

        if(!readInt16BE(file, &readchr))
                return false;
        pm->currentHP = readchr;

        if(!readInt16BE(file, &readchr))
                return false;
        pm->maximumHP = readchr;

        LPMREADBYTE;
        pm->currentMP = readchr;

        LPMREADBYTE;
        pm->maximumMP = readchr;

        LPMREADBYTE;
        pm->padding1[0] = readchr;

        LPMREADBYTE;
        pm->padding1[1] = readchr;

        for(int i = 0; i < 4; i++) {
                LPMREADBYTE;
                pm->items[i] = readchr;
        }

        for(int i = 0; i < 4; i++) {
                LPMREADBYTE;
                pm->magic[i] = readchr;
        }

        for(int i = 0; i < 6; i++) {
                LPMREADBYTE;
                pm->padding2[i] = readchr;
        }

        return true;
}

void PrintCharacterStats2(struct partymember* pm) {
        fprintf(stdout, "Name: %s\n", pm->name);
        fprintf(stdout, "Class:   %3i        Level:   %3i\n",
                pm->class, pm->level);
        fprintf(stdout, "Attack:  %3i        Defence: %3i\n",
                pm->attack, pm->defence);
        fprintf(stdout, "Agility: %3i        Move:    %3i\n",
                pm->agility, pm->movement);
        fprintf(stdout, "Crit?:   %3i        Exp:     %3i\n",
                pm->critical, pm->experience);
        fprintf(stdout, "HP:      %3i/%3i    MP:      %3i/%3i\n",
                pm->currentHP, pm->maximumHP,
                pm->currentMP, pm->maximumMP);

        fprintf(stdout, "??: %3i\n", pm->padding1[0]);
        fprintf(stdout, "??: %3i\n", pm->padding1[1]);

        fprintf(stdout, "Item 1:  %3i\n", pm->items[0]);
        fprintf(stdout, "Item 2:  %3i\n", pm->items[1]);
        fprintf(stdout, "Item 3:  %3i\n", pm->items[2]);
        fprintf(stdout, "Item 4:  %3i\n", pm->items[3]);

        fprintf(stdout, "Magic 1: %3i\n", pm->magic[0]);
        fprintf(stdout, "Magic 2: %3i\n", pm->magic[1]);
        fprintf(stdout, "Magic 3: %3i\n", pm->magic[2]);
        fprintf(stdout, "Magic 4: %3i\n", pm->magic[3]);

        fprintf(stdout, "??: %3i\n", pm->padding2[0]);
        fprintf(stdout, "??: %3i\n", pm->padding2[1]);
        fprintf(stdout, "??: %3i\n", pm->padding2[2]);
        fprintf(stdout, "??: %3i\n", pm->padding2[3]);
        fprintf(stdout, "??: %3i\n", pm->padding2[4]);
        fprintf(stdout, "??: %3i\n", pm->padding2[5]);
}

#define MAXPARTY 30

int main(int argc, char *argv[]) {
        if(argc != 2) {
                fprintf(stderr, "Expecting 1 argument\n");
                return 1;
        }

        FILE* file = fopen(argv[1],"rb");
        if(file == NULL) {
                fprintf(stderr, "%s: %s: %s\n",
                        argv[0], argv[1], strerror(errno));
                return 1;
        }

        struct partymember* pm = CreatePartyMember();

        fseek(file, 0xC10A, 0);
        for(int i = 0; i < MAXPARTY; i++) {
                if(i != 0)
                        fprintf(stdout, "  ------\n");

                if(LoadPartyMember(file, pm)) {
                        PrintCharacterStats2(pm);
                        continue;
                }

                int err = ferror(file);
                if(err != 0)
                                fprintf(stderr, "%s: %s: %s\n",
                                                argv[0], argv[1], strerror(err));
                else
                                fprintf(stderr, "%s: %s: Unexpected EOF\n",
                                                argv[0], argv[1]);
                return 1;
        }
        fclose(file);

        FreePartyMember(pm);
        free(pm);

        return 0;
}
