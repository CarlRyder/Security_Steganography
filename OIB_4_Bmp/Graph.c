// Made by Y. Sendov. April 2022

#define _CRT_SECURE_NO_WARNINGS
#define DEFAULT_ERROR -1
#define BYTES_BMP_SERVICE 54
#define BITS_IN_BYTE 8
#define MAX_BYTE_VALUE 255
#define MAX_DIGIT_INPUT 3
#define MAX_NUMBER_INPUT 23

#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <malloc.h>

int security(char input[MAX_DIGIT_INPUT])
{
	int flag = 0;
	if (input[0] == '\n') return 1;
	input[strcspn(input, "\n")] = 0;
	for (unsigned int i = 0; i < strlen(input); i++)
	{
		if (input[i] < 48 || input[i] > 57)
		{
			flag = 1;
			break;
		}
	}
	return flag;
}

int check_degree(int number)
{
	int flag = 0;
	int degrees[4] = { 1, 2, 4, 8 };
	for (int i = 0; i < 4; i++)
	{
		if (number == degrees[i]) flag = 1;
	}
	return flag;
}

void input_degree(int* degree)
{
	printf("\nEnter the encoding degree value (1, 2, 4, 8): ");
	char input[MAX_DIGIT_INPUT];
	fgets(input, MAX_DIGIT_INPUT, stdin);
	input[strcspn(input, "\n")] = 0;
	fseek(stdin, 0, SEEK_END);
	if (security(input) == 0 && check_degree(atoi(input)) == 1) *degree = atoi(input);
	else
	{
		printf("You entered the encoding degree incorrectly! Try again.\n");
		input_degree(degree);
	}
}

void masks(int degree, unsigned char* text_mask, unsigned char* bmp_mask)
{
	// Calculating the text mask
	if (text_mask != NULL)
	{
		*text_mask = MAX_BYTE_VALUE;
		*text_mask <<= BITS_IN_BYTE - degree;
	}
	// Calculating the mask for an image
	*bmp_mask = MAX_BYTE_VALUE;
	*bmp_mask <<= degree;
}

int get_size(FILE* pointer)
{
	fseek(pointer, 0, SEEK_END);
	int size = ftell(pointer);
	rewind(pointer);
	return size;
}

void coding()
{
	FILE* begin_bmp = fopen("paint.bmp", "rb");
	FILE* coded_bmp = fopen("coded.bmp", "wb");
	FILE* text = fopen("text.txt", "rb");
	if (begin_bmp == NULL || coded_bmp == NULL || text == NULL)
	{
		system("cls");
		printf("File Opening error.\nFiles were expected: \"text.txt\", \"paint.bmp\".\nCheck the source directory!\n");
		exit(DEFAULT_ERROR);
	}
	// Getting the size of the encoded message
	int size_text = get_size(text);
	// Getting the size of the original BMP file
	int size_bmp = get_size(begin_bmp);
	// Entering the encoding degree
	int degree = 0;
	input_degree(&degree);
	if (size_text >= size_bmp * degree / BITS_IN_BYTE - BYTES_BMP_SERVICE)
	{
		system("cls");
		printf("The encoded message is too large for the selected BMP file.\n"
			"Reduce the encoded message in the file \"text.txt\"\n"
			"Or increase the size of the BMP file \"paint.bmp\"\n\n");
		return;
	}
	// Overwriting the first 54 service bytes into an encoded BMP file
	unsigned char* buf_first = malloc(sizeof(unsigned char) * BYTES_BMP_SERVICE);
	if (buf_first != NULL)
	{
		size_t info_bytes = fread(buf_first, 1, BYTES_BMP_SERVICE, begin_bmp);
		if (info_bytes != BYTES_BMP_SERVICE)
		{
			system("cls");
			printf("Error reading data from a file.\nCheck the original BMP file!\n");
			exit(DEFAULT_ERROR);
		}
		fwrite(buf_first, 1, BYTES_BMP_SERVICE, coded_bmp);
		free(buf_first);
	}
	// Creating bit masks
	unsigned char text_mask = 0;
	unsigned char bmp_mask = 0;
	masks(degree, &text_mask, &bmp_mask);
	// Writing encoded bytes to a new image
	unsigned char* buf_img = malloc(sizeof(unsigned char));
	unsigned char* buf_txt = malloc(sizeof(unsigned char));
	unsigned char* buffer = malloc(sizeof(unsigned char));
	if (buf_img != NULL && buf_txt != NULL && buffer != NULL)
	{
		int counter = 0;
		while (1)
		{
			size_t symbol = fread(buffer, 1, 1, text);
			if (counter == size_text) break;
			for (int bit = 0; bit < BITS_IN_BYTE; bit += degree)
			{
				size_t read = fread(buf_img, 1, 1, begin_bmp);
				unsigned char byte = buf_img[0] & bmp_mask;
				unsigned char bits = buffer[0] & text_mask;
				bits >>= BITS_IN_BYTE - degree;
				byte |= bits;
				buf_txt[0] = byte;
				fwrite(buf_txt, 1, 1, coded_bmp);
				buffer[0] <<= degree;
				buf_img[0] = 0;
				buf_txt[0] = 0;
			}
			buffer[0] = 0;
			counter++;
		}
		free(buf_img);
		free(buf_txt);
		free(buffer);
	}
	// Writing the remaining bytes to the encoded BMP file
	int count = size_bmp - ftell(begin_bmp);
	unsigned char* buf_last = malloc(sizeof(unsigned char) * count);
	if (buf_last != NULL)
	{
		size_t last_bytes = fread(buf_last, 1, count, begin_bmp);
		if (last_bytes != count)
		{
			system("cls");
			printf("Error reading data from the file.\nCheck the original BMP file!\n");
			exit(DEFAULT_ERROR);
		}
		fwrite(buf_last, 1, count, coded_bmp);
		free(buf_last);
	}
	fclose(text);
	fclose(begin_bmp);
	fclose(coded_bmp);
	// Successful exit from the function
	system("cls");
	printf("Your text has been successfully encoded in the file \"coded.bmp\"\nLength of the encoded message - %d bytes\n\n", size_text);
}

void input_count(int* count)
{
	printf("\nEnter the number of bytes of the secret message to be decoded: ");
	char input[MAX_NUMBER_INPUT];
	fgets(input, MAX_NUMBER_INPUT, stdin);
	input[strcspn(input, "\n")] = 0;
	fseek(stdin, 0, SEEK_END);
	if (security(input) == 0) *count = atoi(input);
	else
	{
		printf("You entered the number of bytes incorrectly! Try again.\n");
		input_count(count);
	}
}

void decoding()
{
	FILE* coded_bmp = fopen("coded.bmp", "rb");
	FILE* end_text = fopen("decoded.txt", "wb");
	if (coded_bmp == NULL || end_text == NULL)
	{
		system("cls");
		printf("Error opening files.\nFiles were expected: \"decoded.txt\", \"coded.bmp\".\nCheck the source directory!\n");
		exit(DEFAULT_ERROR);
	}
	// Request for the number of characters in the message to be decoded
	int read_count = 0;
	input_count(&read_count);
	// Getting the size of the original BMP file
	int size_bmp = get_size(coded_bmp);
	// Entering the encoding degree
	int degree = 0;
	input_degree(&degree);
	if (read_count >= size_bmp * degree / BITS_IN_BYTE - BYTES_BMP_SERVICE)
	{
		system("cls");
		printf("The encoded message is too large for the selected BMP file.\n"
			"Reduce the encoded message in the file \"text.txt\"\n"
			"Or increase the size of the BMP file \"paint.bmp\"\n\n");
		return;
	}
	// We skip the first 54 service bytes
	fseek(coded_bmp, BYTES_BMP_SERVICE, SEEK_SET);
	// Creating a bit mask
	unsigned char bmp_mask = 0;
	masks(degree, NULL, &bmp_mask);
	bmp_mask = ~bmp_mask; // We take the reverse mask
	// Reading the necessary characters
	unsigned char* buf_img = malloc(sizeof(unsigned char));
	unsigned char* buffer = malloc(sizeof(unsigned char));
	if (buf_img != NULL && buffer != NULL)
	{
		int read = 0;
		while (read < read_count)
		{
			unsigned char symbol = 0;
			for (int bit = 0; bit < BITS_IN_BYTE; bit += degree)
			{
				size_t read = fread(buf_img, 1, 1, coded_bmp);
				unsigned char byte = buf_img[0] & bmp_mask;
				symbol <<= degree;
				symbol |= byte;
				buf_img[0] = 0;
			}
			buffer[0] = symbol;
			fwrite(buffer, 1, 1, end_text);
			read++;
			buffer[0] = 0;
		}
		free(buf_img);
		free(buffer);
	}
	fclose(coded_bmp);
	fclose(end_text);
	// Successful exit from the function
	system("cls");
	printf("Your text has been successfully decoded into a file \"decoded.txt\"\n\n");
}

int menu()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	printf("Application Menu \"BMP Encoder\"\n\n"
		"1. Encoding a BMP file\n"
		"2. Decoding a BMP file\n"
		"3. Exiting the program\n"
		"\nSelect the number of the program's operating mode: ");
	int value = -1;
	char input[MAX_DIGIT_INPUT];
	fgets(input, MAX_DIGIT_INPUT, stdin);
	input[strcspn(input, "\n")] = 0;
	fseek(stdin, 0, SEEK_END);
	value = atoi(input);
	if (security(input) == 0 && value < 4) return value;
	else return -1;
}

int main()
{
	int n = menu();
	while (1)
	{
		switch (n)
		{
		case 1:
			coding();
			break;
		case 2:
			decoding();
			break;
		case 3:
			system("cls");
			printf("You have exited the program!\n");
			return 0;
		default:
			system("cls");
			printf("Incorrect input. Try again!\n\n");
			break;
		}
		n = menu();
	}
}