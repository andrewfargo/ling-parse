#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct verbal_entry {
	char word[256];
	char lemma[256];
	long occurences;
} verbal_entry_t;

typedef struct written_post {
	long byte_pos;
	char word[256];
} written_post_t;

typedef struct post_range {
	long beg;
	long end;
} post_range_t;

written_post_t posts[1012];


int load_verbal_entry(FILE* file, verbal_entry_t *entry)
{
	char *entry_s = malloc(sizeof(char) * 256);
	fgets(entry_s, 256, file);

	char *word = strtok(entry_s, "\t");
	if (!word)
	{
		fprintf(stderr, "Couldn't read word from string.\n");
		return -1;
	}

	strcpy(entry->word, word);

	char *lemma = strtok(NULL, "\t");

	if (!lemma)
	{
		fprintf(stderr, "Couldn't read lemma from string.\n");
		return -1;
	}

	strcpy(entry->lemma, lemma);

	strtok(NULL, "\t");

	char *occurences = strtok(NULL, "\t");

	if (!occurences)
	{
		fprintf(stderr, "Couldn't read occurences from string.\n");
		return -1;
	}

	occurences[strlen(occurences)-1] = '\0'; // trim the newline

	entry->occurences = atol(occurences);

	free(entry_s);
	return 0;
}

int load_posts(FILE *file)
{
	for (int i = 0; i < 1012; i++)
	{
		char post_entry_s[256];
		fgets(post_entry_s, 256, file);
		
		posts[i].byte_pos = atol(strtok(post_entry_s, "\t"));

		char *word = strtok(NULL, "\t");

		strcpy(posts[i].word, word);

	}
	printf("Precomputation posts loaded.\n");
	return 0;
}

post_range_t get_post_range(char *word) {
	// modified binary search
	int min = 0;
	int max = 1011;
	while (max - min > 1)
	{
		int try = ((max - min) / 2) + min;
		int compare = strcmp(posts[try].word, word);
		if (compare < 0) {
			min = try;
		} else if (compare > 0) {
			max = try;
		} else { // rare case
			min = try;
			max = min + 1;
		}
	}

	post_range_t range;
	range.beg = posts[min].byte_pos;
	range.end = posts[max].byte_pos;
	return range;
}

int main(int argc, char** argv) {

	// Algorithm
	// Pick a spoken word from occurance data
	// 	Occurance / Total = Verbal Usage Ratio
	// Take word's lemma and find in written lemma data
	// 	Occurance / Total = Written Usage Ratio
	// Find difference in ratios -> log into file
	// Repeat for arbitrary amount of words
	
	int n = 100000;

	FILE *verbal_freq = fopen("ANC-spoken-count.txt", "r");
	FILE *written_lemma = fopen("ANC-written-lemma.txt", "r");

	if (!(verbal_freq && written_lemma))
	{
		fprintf(stderr, "Couldn't open data files.\n");
		return -1;
	}

	FILE *result_log = fopen("results.txt", "a+");

	if(!result_log)
	{
		fprintf(stderr, "Couldn't create results file.\n");
		return -1;
	}

	FILE *post_output = fopen("post-output.txt", "r");

	if (!post_output)
	{
		fprintf(stderr, "Couldn't read post-output file, compile and run `post.c`.");
		return -1;
	}

	load_posts(post_output);

	for(int i = 0; i < n; i++)
	{
		verbal_entry_t *entry = malloc(sizeof(verbal_entry_t));
		int status = load_verbal_entry(verbal_freq, entry);

		if (status != 0)
		{
			return status;
		}

		float ratio_s = ((float)(entry->occurences)) / (float)(3863592);

		// find word in written record
		post_range_t byte_range = get_post_range(entry->word);
		fseek(written_lemma, byte_range.beg, SEEK_SET);

		long written_occurences = 0;

		while (ftell(written_lemma) < byte_range.end)
		{
			char written_entry_s[256];
			fgets(written_entry_s, 256, written_lemma);
			int compare = strncmp(entry->word, written_entry_s, strlen(entry->word));
			if (compare != 0)
				continue;
			// word found! parse
			strtok(written_entry_s, "\t");
			strtok(NULL, "\t");
			strtok(NULL, "\t");
			written_occurences = atol(strtok(NULL, "\t"));
		}

		float ratio_w = ((float)(written_occurences)) / (float)(22393704);

		if (ratio_w == 0)
			continue;

		float percent_change = (ratio_s - ratio_w) / ratio_w;

		if (percent_change < -0.98)
		{
			fprintf(result_log, "%s & %4.1f\\%%\\\\\n", entry->word, percent_change * 100);
		}
	}


	fclose(verbal_freq);
	fclose(written_lemma);
	fclose(result_log);
	fclose(post_output);
	return 0;
}
