// void copyString(char* dest, const char* str)
// {
//     // while (*str != 0)
//     {
//         *dest = *str;
//         dest += 2;
//         str++;
//     }
// }

void start() 
{
    char* video_memory = (char*) 0xb8000;

    // Clear screen
    for (int i=0; i<80*25*2; i+=2)
    {
        video_memory[i] = ' ';
        video_memory[i+1] = 0x0F; // Light grey on black
    }

    // Output string
    video_memory[0] = 'T';
    video_memory[2] = 'O';
    video_memory[4] = 'S';
    // copyString(video_memory, "Thijs");
}
