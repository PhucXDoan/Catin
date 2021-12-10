#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define false 0
#define true  1

#define ASSERT(condition) do { if (!(condition)) { *(int32_t*) (0) = 0; } } while (false)
#define ARRAY_COUNT(xs) (sizeof(xs) / sizeof((xs)[0]))
#define IN_RANGE(x, minimum, maximum) ((minimum) <= (x) && (x) < (maximum))
#define FOR_RANGE(name, minimum, maximum) for (i32 (name) = (minimum); (name) < (maximum); ++(name))

#define MAX_ENTRY_COUNT      256
#define MAX_SLOT_COUNT       8
#define MAX_SLOT_SIZE        64
#define MAX_ENTRY_SIZE       (MAX_SLOT_COUNT * MAX_SLOT_SIZE)
#define DICTIONARY_FILE_PATH "W:/src/dictionary.txt"

typedef const char* cstr;
typedef int32_t     i32;
typedef int32_t     bool32;

typedef enum
{
	query_flag_unprompted = 0,
	query_flag_incorrect,
	query_flag_correct
} query_flag_t;

void extract_slot(char* hidden_entry_buffer, i32 hidden_entry_size, char* hidden_slot_buffer, i32 hidden_slot_size, cstr entry, i32 slot_index)
{
	ASSERT(slot_index >= 0);

	i32 current_slot_index = 0;
	i32 char_index         = 0;

	while (current_slot_index < slot_index || (current_slot_index == slot_index && entry[char_index] == ' '))
	{
		ASSERT(entry[char_index] != '\0');

		if (entry[char_index] == ',')
		{
			++current_slot_index;
		}

		hidden_entry_buffer[char_index] = entry[char_index];
		++char_index;
	}

	bool32 is_end_of_slot_found = false;
	FOR_RANGE(i, 0, hidden_slot_size)
	{
		if (entry[char_index] == ',' || entry[char_index] == '\0')
		{
			hidden_slot_buffer[i] = '\0';
			is_end_of_slot_found = true;
			break;
		}
		else
		{
			hidden_slot_buffer[i]           = entry[char_index];
			hidden_entry_buffer[char_index] = '_';
			++char_index;
		}
	}

	while (true)
	{
		hidden_entry_buffer[char_index] = entry[char_index];

		if (entry[char_index] == '\0')
		{
			break;
		}
		else
		{
			++char_index;
		}
	}

	ASSERT(is_end_of_slot_found);
}

i32 count_entry_slots(cstr entry)
{
	i32 counter = 1;
	for (i32 i = 0; entry[i] != '\0'; ++i)
	{
		if (entry[i] == ',')
		{
			++counter;
		}
	}
	return counter;
}

i32 rand_index(i32 length)
{
	return rand() % length; // @TODO@ Make this better!
}

int main(void)
{
	srand((i32) time(0)); // @TODO@ Had to include `<time.h>` just for this! Ew!

	char  entry_char_buffer[MAX_ENTRY_COUNT * MAX_ENTRY_SIZE];
	char* entry_buffer[MAX_ENTRY_COUNT];
	i32   entry_count = 0;
	{
		FILE* dictionary_file;
		fopen_s(&dictionary_file, DICTIONARY_FILE_PATH, "r");

		if (dictionary_file)
		{
			entry_buffer[0] = entry_char_buffer;

			while (true)
			{
				char buffer[MAX_ENTRY_SIZE];

				fgets(buffer, ARRAY_COUNT(buffer), dictionary_file);

				if (feof(dictionary_file))
				{
					break;
				}
				else
				{
					bool32 is_newline_found = false;

					FOR_RANGE(i, 0, ARRAY_COUNT(buffer))
					{
						if (buffer[i] == '\n')
						{
							entry_buffer[entry_count][i] = '\0';
							is_newline_found = true;
							++entry_count;
							entry_buffer[entry_count] = entry_buffer[entry_count - 1] + i + 1;
							break;
						}
						else
						{
							entry_buffer[entry_count][i] = buffer[i];
						}
					}

					ASSERT(is_newline_found);
					ASSERT(IN_RANGE(entry_count, 0, MAX_ENTRY_COUNT));
				}
			}

			fclose(dictionary_file);
		}
		else
		{
			printf("Failed to open '%s'.\n", DICTIONARY_FILE_PATH);
			return -1;
		}
	}

	query_flag_t flag_buffer[MAX_ENTRY_COUNT] = { 0 };
	i32          unprompted_count             = entry_count;
	i32          incorrect_count              = 0;

	while (true)
	{
		if (unprompted_count)
		{
			i32  unprompted_index = rand_index(unprompted_count);
			i32  entry_index      = -1;
			cstr entry            = 0;

			{
				i32 unprompted_counter = -1;
				FOR_RANGE(i, 0, entry_count)
				{
					if (flag_buffer[i] == 0)
					{
						++unprompted_counter;
					}

					if (unprompted_counter == unprompted_index)
					{
						entry_index = i;
						entry       = entry_buffer[i];

						break;
					}
				}

				ASSERT(entry);
			}

			char hidden_entry_buffer[MAX_ENTRY_SIZE];
			char hidden_slot_buffer[MAX_SLOT_SIZE];
			extract_slot(hidden_entry_buffer, ARRAY_COUNT(hidden_entry_buffer), hidden_slot_buffer, ARRAY_COUNT(hidden_slot_buffer), entry, rand_index(count_entry_slots(entry)));
			printf("%s : ", hidden_entry_buffer);

			char input_buffer[MAX_SLOT_SIZE];
			fgets(input_buffer, ARRAY_COUNT(input_buffer), stdin);

			if (feof(stdin))
			{
				printf("EOF in STDIN.\n");
				return -1;
			}
			else
			{
				{
					bool32 is_newline_found = false;
					FOR_RANGE(i, 0, ARRAY_COUNT(input_buffer))
					{
						if (input_buffer[i] == '\n')
						{
							is_newline_found = true;
							input_buffer[i] = '\0';
							break;
						}
					}
					ASSERT(is_newline_found);
				}

				printf("%s : %s\n", entry, hidden_slot_buffer);

				bool32 is_correct = true;
				{
					i32 input_char_index = 0;
					FOR_RANGE(i, 0, ARRAY_COUNT(hidden_slot_buffer))
					{
						if (input_char_index == -1)
						{
							if (hidden_slot_buffer[i] == '/')
							{
								input_char_index = 0;
							}
							else if (hidden_slot_buffer[i] == '\0')
							{
								is_correct = false;
								break;
							}
						}
						else if (input_buffer[input_char_index] == '\0' && (hidden_slot_buffer[i] == '\0' || hidden_slot_buffer[i] == '/'))
						{
							break;
						}
						else if (hidden_slot_buffer[i] == '\0')
						{
							is_correct = false;
							break;
						}
						else if (input_buffer[input_char_index] == hidden_slot_buffer[i])
						{
							++input_char_index;
						}
						else
						{
							input_char_index = -1;
						}
					}
				}

				if (is_correct)
				{
					printf("Correct! %d left.\n\n", unprompted_count - 1);
					flag_buffer[entry_index] = query_flag_correct;
				}
				else
				{
					printf("Incorrect... %d left.\n\n", unprompted_count - 1);
					flag_buffer[entry_index] = query_flag_incorrect;
					++incorrect_count;
				}

				--unprompted_count;
			}
		}
		else if (incorrect_count)
		{
			printf("Reviewing %d entries!\n\n", incorrect_count);
			FOR_RANGE(i, 0, entry_count)
			{
				flag_buffer[i] = flag_buffer[i] == query_flag_incorrect ? query_flag_unprompted : query_flag_correct;
			}
			unprompted_count = incorrect_count;
			incorrect_count  = 0;
		}
		else
		{
			printf("Shuffling...\n\n");
			FOR_RANGE(i, 0, entry_count)
			{
				flag_buffer[i] = 0;
			}
			unprompted_count = entry_count;
		}
	}

	return 0;
}
