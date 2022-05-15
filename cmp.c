#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **original_file_strings = NULL;
size_t total_lines_original = 0;

FILE *outfile;
FILE *outjson;
char *file_type;

int comparator(const void *a, const void *b) {
	char *a_in = (char *) a;
	char *b_in = * (char **) b;
	return strcmp(a_in, b_in);
}

void parse_line(char *);
char *fix_coords(char *);

int main(int argc, char **argv) {

	//  1. Open the files
	if (argc < 4) {
		if (argc > 1) printf("Error: No Comparison File Given\n");
		else if (argc > 2) printf("Error: Specify type of file (billboard or statue)\n");
		else printf("Error: No Files given\n");
		printf("Usage: ./cmp <original-file> <race-event-file> <filetype>\n");
		return -1;
	}
	
	char *original_file = argv[1];
	char *check_file = argv[2];
	if (!strcmp(argv[3], "billboard") && !strcmp(argv[3], "statue")) {
		printf("Incorrect filetype, please specify either 'billboard' or 'statue'\n");
		return -1;
	}
	file_type = argv[3];

	FILE *origin_fp = fopen(original_file, "r");
	FILE *check_fp = fopen(check_file, "r");
	outfile = fopen("standard_output.txt", "w");
	outjson = fopen("json_output.txt", "w");

	if (!origin_fp || !check_fp || !outfile) {
		printf("Error Opening File.\n");
		return -1;
	}

	char *origin_lineptr = NULL;
	size_t origin_size = 0;
	char *race_event_lineptr = NULL;
	size_t race_event_size = 0;

	//       2. Read lines from the original file...         
	while (getline(&origin_lineptr, &origin_size, origin_fp) != -1) {

		// reallocate array
		original_file_strings = realloc(original_file_strings, sizeof(char*) * (total_lines_original + 1));

		// allocate memory for line
		original_file_strings[total_lines_original] = malloc(strlen(origin_lineptr) + 1);
		strcpy(original_file_strings[total_lines_original], origin_lineptr);
		total_lines_original++;
	}

	fprintf(outfile, "Different Line Outputs:\n");
	fprintf(outjson, "Different Line Outputs:\n");

	//		3. Read lines from the second file, see if it's in the original file
	while (getline(&race_event_lineptr, &race_event_size, check_fp) != -1) {

		// search original file strings
		char **result = lfind(race_event_lineptr, original_file_strings, &total_lines_original, sizeof(char*), comparator);
		
		if (result) {
			// get index and splice it from the original lines array
			int index = result - original_file_strings;
			free(original_file_strings[index]);
			for (int i = index; i < total_lines_original - 1; i++) original_file_strings[i] = original_file_strings[i + 1];
        	total_lines_original--;

			// reallocate the array
			original_file_strings = realloc(original_file_strings, sizeof(char*) * (total_lines_original + 1));
			continue;
		}

		parse_line(race_event_lineptr);
	}

	//		4. Free each element in the string array
	for (int i = 0; i < total_lines_original; i++) {
		free(original_file_strings[i]);
	}


	//       5. Clean up the rest
	free(original_file_strings);
	free(origin_lineptr);
	free(race_event_lineptr);

	fclose(origin_fp);
	fclose(check_fp);
	fclose(outfile);
	fclose(outjson);
	return 0;
}

/*
#################################################
PARSE LINE TAKES THE NEW LINE, AND WRITES TO FILE
#################################################
*/
// Standard Line for statues / billboards : [x y z] arg1 arg2 arg3 arg4 etc
void parse_line(char *line) {

	// Write line to standard output
	size_t line_len = strlen(line);
	fwrite(line, line_len, 1, outfile);

	char *stringp = line;

	// remove newline character
	stringp[strlen(stringp) - 1] = '\0';
	
	char *bracketptr = stringp;
	// keep moving the pointer forward by 1 until we reach the end bracket
	while (bracketptr != strstr(bracketptr, "]"))
		bracketptr++;
	int index = bracketptr - stringp + 1;

	// duplicate the coords including end bracket
	char *coords = strndup(stringp, index);
	// add commas
	coords = fix_coords(coords);
	// output coords
	fprintf(outjson, "{\n\tcoords: %s,\n\t", coords);

	// move string pointer by 1 after index
	stringp += index + 1;

	char *token = NULL;
	if (strcmp("billboard", file_type) == 0) {
		token = strsep(&stringp, " ");
		fprintf(outjson, "size: %s,\n\t", token);

		token = strsep(&stringp, " ");
		fprintf(outjson, "aspect: %s,\n\t", token);

		token = strsep(&stringp, " ");
		fprintf(outjson, "png: \"%s\"\n", token);
	} else {
		token = strsep(&stringp, " ");
		fprintf(outjson, "angle: %s,\n\t", token);

		token = strsep(&stringp, " ");
		fprintf(outjson, "jm: \"%s\",\n\t", token);

		token = strsep(&stringp, " ");
		fprintf(outjson, "png: \"%s\",\n\t", token);

		token = strsep(&stringp, " ");
		fprintf(outjson, "shp: \"%s\"\n", token);
	}

	fprintf(outjson, "},\n");
	free(coords);
}

char *fix_coords(char *text) {
	char *ptr;
	// replace whitespace with comma with memcpy
	for (ptr = strstr(text, " "); ptr; ptr = strstr(ptr, " ")) {
		memcpy(&ptr[0], ",", 1);
	}
	return text;
}
