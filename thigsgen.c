#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#include "thigsgen.h"

FILE *txt;
FILE *save_file;

char save_file_name[15];
char * save_file_name_start = "TONYHAXINTGS0"; // TONYHAXINTGS14 is the longest possible string (14 bytes + 1 for termination = 15).

uint8_t *bytes;
uint8_t gameshark_code_type;
uint8_t number_of_gameshark_code_lines = 0;
uint8_t gameshark_patch_code_number_of_codes;
uint8_t gameshark_patch_code_offset_to_add_to_each_code_address;
uint8_t sum;
uint8_t prev = 0;
uint8_t next;
uint8_t save_file_number = 0; // Start with 0. 0-14 range.
uint8_t save_file_number_val = 0; // ASCII conversion holder.

uint16_t gameshark_patch_code_offset_to_add_to_each_code_write_val;
uint16_t gameshark_code_address_mod_val;

int32_t read_codes;
uint32_t gameshark_code_address;

void do_exit(unsigned char exit_code)
{
    #if defined WIN32
        #include <conio.h>
        printf("\nPress any key to continue...\n"); // Because system("pause") sucks we can do better.
        _getch();
    #endif
    exit(exit_code);
}

bool parse()
{
    printf("GS Code Address/Prefix: %08X\n", gameshark_code_address);
    bytes = (uint8_t *)&gameshark_code_address;
    uint8_t gameshark_code_type = bytes[3];

    if(
        (gameshark_code_type != 0x50) && // Storage type for multiple codes.

        (gameshark_code_type != 0x80) && // 16 bit codes.
        (gameshark_code_type != 0xD0) && 
        (gameshark_code_type != 0xD1) && 
        (gameshark_code_type != 0xD2) && 
        (gameshark_code_type != 0xD3) &&
    
        (gameshark_code_type != 0x30) && // 8 bit codes,
        (gameshark_code_type != 0xE0) && 
        (gameshark_code_type != 0xE1) && 
        (gameshark_code_type != 0xE2) && 
        (gameshark_code_type != 0xE3)
        )
    {
        printf("Error: unsupported code type: %02X, %s was NOT generated, bailing...\n", gameshark_code_type, save_file_name);
        fclose(txt);
        fclose(save_file);
        remove(save_file_name);
        return false;
    }

    if(gameshark_code_type == 0x50)
    {
        gameshark_patch_code_number_of_codes = bytes[1];
        printf("Number of 80 codes to be derived from 50 type code: 0x%02X\n", gameshark_patch_code_number_of_codes);
        gameshark_patch_code_offset_to_add_to_each_code_address = bytes[0];
        printf("Offset to add to each 80 code address: 0x%02X\n", gameshark_patch_code_offset_to_add_to_each_code_address);
        printf("Offset to add to each write val: 0x%hX\n", gameshark_patch_code_offset_to_add_to_each_code_write_val);
        fscanf(txt, "%08X %hX\n", &gameshark_code_address, &gameshark_code_address_mod_val); // Get real vals from next line.

        for (int i = 0; i < gameshark_patch_code_number_of_codes; i++)
        {
            printf("GS Code Address/Prefix: %08X\n", gameshark_code_address);
            printf("GS Code Mod Value: %02hX\n", gameshark_code_address_mod_val);
            fwrite(&gameshark_code_address, 1, 4, save_file);
            fwrite(&gameshark_code_address_mod_val, 1, 2, save_file);
            gameshark_code_address = (gameshark_code_address + gameshark_patch_code_offset_to_add_to_each_code_address);
            gameshark_code_address_mod_val = (gameshark_code_address_mod_val + gameshark_patch_code_offset_to_add_to_each_code_write_val);
            number_of_gameshark_code_lines++;
        }
    } else {
        fwrite(&gameshark_code_address, 1, 4, save_file);
        printf("GS Code Type/Prefix: 0x%02X\n", gameshark_code_type);
        printf("GS Code Mod Value: %hX\n", gameshark_code_address_mod_val);
        fwrite(&gameshark_code_address_mod_val, 1, 2, save_file);
        number_of_gameshark_code_lines++;
    }

    return true;
}

void increase_save_file_name()
{
    save_file_number++;
    save_file_number_val = save_file_number;

    // Get ascii for string append.
    if(save_file_number < 10) // Single digit handling.
    {
        save_file_number_val += 0x30; // add 0x30 for 0-9. Ghetto ASCII conversion.
        save_file_name[12] = save_file_number_val; // Single digit means we only modify byte 12.
        save_file_name[13] = '\0'; // Null terminate byte 13.
    } else {
        save_file_name[12] = 0x31; // 1 is always the first digit in a double digit number.

        if(save_file_number == 10) { // Double digit value handling involes modifying byte 13.
            save_file_name[13] = 0x30; // 0	for 10.		
        } else if(save_file_number == 11) {
            save_file_name[13] = 0x31; // 1 for 11.	
        } else if(save_file_number == 12) {
            save_file_name[13] = 0x32; // 2	for 12.	
        } else if(save_file_number == 13) {
            save_file_name[13] = 0x33; // 3 for 13.
        } else if(save_file_number == 14) {
            save_file_name[13] = 0x34; // 4	for 14.		
        }
        
        save_file_name[14] = '\0'; // Add null termination.
    }
}

void test_open_save_file()
{
    save_file = fopen(save_file_name, "r");
}

int main(int argc, const char *argv[])
{
    printf("Tonyhax International GameShark GENerator (THIGSGEN) %s\n(C)2023-2025, Alex Free (3-BSD)\n", VERSION);

    if(argc != 2)
    {
        printf("Error: THIGSGEN requires 1 argument.\nUsage:\n\nthigsgen <txt file of GameShark codes>\n\n<txt file of game shark codes>      This should have each GameShark code on it's own line. No blank lines may seperate each code.");
        do_exit(1);
    }

    if((txt = fopen(argv[1], "r")) == NULL)
    {
        printf("Error: cannot read the input txt file: %s.\n", argv[1]);
        do_exit(1);
    }
    
    for (int i = 0; i < 13; i++) {
        save_file_name[i] = save_file_name_start[i];
    }

    test_open_save_file(); // Read only test for if it exists already.

    for(int i = 0; i < 14; i++) // 0-14 range.
    {
        //printf("%s\n", save_file_name);

        if(save_file)
        {
            fclose(save_file);
        } else {
            break; // Not even TONYHAXINTGS0 exists yet so we are ok.
        }
        
        increase_save_file_name();
        test_open_save_file();
        
        if(!save_file)
        {
            break;
        }
    }

    if(save_file) // If we have a valid file pointer at this point we have too many files.
    {
        printf("Error: you already have 15 different save files in your current folder. Please run thigsgen in a different folder or delete some of the generated save files in this one\n");
        do_exit(1);
    }
    
    // At this point we know we can create a new save file with a new name.

    if((save_file = fopen(save_file_name, "wb+")) == NULL) // Open as read write.
    {
        printf("Error: cannot create %s in the current directory\n", save_file_name);
        fclose(txt);
        do_exit(1);
    }

    printf("Save file name: %s\n", save_file_name);

    fseek(save_file, 0, SEEK_SET);
    fseek(txt, 0, SEEK_SET);

    // Write header stub (unsigned char array from card.bin).
    for (int i = 0; i < 0x2000; i++)
    {
        fputc(0x00, save_file);
    }

    fseek(save_file, 0, SEEK_SET);

    for (int i = 0; i < 0x101; i++)
    {
        fputc(save[i], save_file);
    }

    fseek(save_file, 0x104, SEEK_SET); // GameShark code data starts here.
    printf("\n");

    while (1)
    {
        read_codes = fscanf(txt, "%08X %hX\n", &gameshark_code_address, &gameshark_code_address_mod_val);;

        if(read_codes != EOF && read_codes != 0)
        {
            if(!parse())
            {
                do_exit(1);
            }
        } else {
            printf("\nReading of txt file: %s is complete.\n\n", argv[1]);
            break;
        }
    }

    fseek(save_file, 0x103, SEEK_SET);
    fwrite(&number_of_gameshark_code_lines, 1, 1, save_file);
    fseek(save_file, 0x103, SEEK_SET); // Checksummed data starting here.

    for(int i = 0x103; i < 0x2000; i++)
    {
        next = fgetc(save_file);
        sum = prev + next;
        sum &= 0xFF;
        prev = sum;
    }

    fseek(save_file, 0x102, SEEK_SET); // Checksum value here.
    fwrite(&sum, 1, 1, save_file);
    printf("%s checksum: 0x%02X.\n", save_file_name, sum);

    fclose(save_file);
    fclose(txt);
    printf("Generated Tonyhax International save file named: %s containing %d GameShark code lines.\n", save_file_name, number_of_gameshark_code_lines);
    do_exit(0);
}
