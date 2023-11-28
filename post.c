// TODO: filter out unicode posts in code
// go to next occurence instead

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct written_post {
	long byte_pos;
	char *word;
} written_post_t;

// Subdivide data into 1024 regions and mark which word occurs in which region
// Output to file to be read for precompute

// HACK: Twelve sections contain unicode characters, I've removed them from the record, total is 1012 "pages"

int main(int argc, char **argv) {
	FILE *written_lemma = fopen("ANC-written-lemma.txt", "r");
	FILE *post_output = fopen("post-output.txt", "a+");

	long filesize;
	fseek(written_lemma, 0L, SEEK_END);
	filesize = ftell(written_lemma);

	float post_range = (float)filesize / 1024.0f;

	for (int i = 0; i < 1024; i++)
	{
		long offset = (long)(post_range * (float)i);
		fseek(written_lemma, offset, SEEK_SET);
		char read;

		do {
			read = fgetc(written_lemma);
			offset++;
		} while (read != '\n'); // advance until new line

		if (i == 0) // special case
		{
			offset = 0;
			rewind(written_lemma);
		}

		char word[128];
		int pos = 0;

		do {
			word[pos] = fgetc(written_lemma);
		} while (word[pos++] != '\t');

		word[pos] = '\0';
		
		char entry[256];

		sprintf(entry, "%ld\t%s\n", offset, word);
		printf(entry);
		fprintf(post_output, entry);
	}

	return 0;
}

