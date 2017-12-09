/*
 * libredex.c
 *
 * this file is part of LIBREDEX
 *
 * Copyright (c) 2017 Brett Sheffield <brett@gladserv.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING in the distribution).
 * If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <assert.h>
#include <endian.h>
#include <inttypes.h>
#include <librecast.h>
#include <limits.h>
#include <lmdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cJSON.h"

#ifndef SHA_DIGEST_LENGTH
#define SHA_DIGEST_LENGTH 20
#endif

#define TOKSEP "?!\"'$%^&*()-_=+[]{};:@~#,.<>/\\|` "

int main(int argc, char **argv)
{
	int i;
	int mode;
	int msgs;
	int rc = 0;
	char *key = NULL;
	lc_ctx_t *ctx;
	lc_query_t *q = NULL;
	lc_messagelist_t *msglist = NULL, *msg;
	cJSON *root = NULL;
	cJSON *nick = NULL;
	cJSON *text = NULL;

	ctx = lc_ctx_new();
	rc = lc_query_new(ctx, &q);

	assert(rc == 0);
	assert(q != NULL);

	/* TODO: find messages that haven't been indexed yet */

	/* find highest indexed timestamp in some index */
	/* use that as a starting point */

	uint64_t v = 0; /* <--- TODO: find an appropriate value for this */
	lc_query_push(q, LC_QUERY_TIME | LC_QUERY_GT, &v);
	msgs = lc_query_exec(q, &msglist);

	/* write indexes for messages */
	for (msg = msglist; msg != NULL; msg = msg->next) {
		root = cJSON_Parse(msg->data);
		if (root == NULL)
			continue;

		/* show user which message we're indexing */
		printf("%" PRIu64 " ", msg->timestamp);
		for (i = 0; i < 20; ++i) {
			printf("%02x", ((unsigned char *)msg->hash)[i]);
		}
		printf(" %s\n", (char *)msg->data);

		nick = cJSON_GetObjectItemCaseSensitive(root, "nick");
		text = cJSON_GetObjectItemCaseSensitive(root, "text");
		mode = LC_DB_MODE_DUP | LC_DB_MODE_BOTH;

		/* index by nick */
		if (nick != NULL) {
			rc = lc_db_idx(ctx, "message", "nick", msg->hash, SHA_DIGEST_LENGTH,
				nick->valuestring, strlen(nick->valuestring), mode);
			if (rc != 0)
				fprintf(stderr, "nick index failed\n");
			else
				fprintf(stderr, "\tnick: %s\n", nick->valuestring);
		}

		/* index by keyword */
		key = strtok(text->valuestring, TOKSEP);
		while (key != NULL) {
			fprintf(stderr, "\tkey: %s\n", key);
			rc = lc_db_idx(ctx, "message", "keyword", msg->hash, SHA_DIGEST_LENGTH,
				key, strlen(key), mode);
			if (rc != 0)
				fprintf(stderr, "error writing index key %s\n", key);

			key = strtok(NULL, TOKSEP);
		}

		cJSON_Delete(root);

	}
	printf("Indexed %i msgs\n", msgs);

	/* clean up */
	lc_msglist_free(msglist);
	lc_query_free(q);
	lc_ctx_free(ctx);

	return 0;
}
