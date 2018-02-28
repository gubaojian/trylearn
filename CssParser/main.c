/**
 * ln  -s /opt/netsurf/include/hubbub /usr/local/include/hubbub
 *
 * */

#include <stdio.h>
#include <libwapcaplet/libwapcaplet.h>
#include <libcss/libcss.h>
#include <hubbub/hubbub.h>
#include <hubbub/parser.h>
#include <time.h>




static hubbub_error token_handler(const hubbub_token *token, void *pw);

char* readFile(char const* fileName){

    FILE* fp;
    fp = fopen(fileName, "r");
    if(!fp){
        printf("error open %s", fileName);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    printf("%s file length %lu \n ", fileName, length);
    char* buffer = (char*)malloc(sizeof(char)*length);
    while(length > 0){
        int read = fread(buffer, sizeof(char), length, fp);
        length -=read;
    }
    fclose(fp);
    return buffer;
}

int main() {
    hubbub_parser *parser;
    hubbub_parser_optparams params;


    assert(hubbub_parser_create("UTF-8", false, &parser) == HUBBUB_OK);

    params.token_handler.handler = token_handler;
    params.token_handler.pw = NULL;




    char* utf8Html = readFile("/Users/furture/code/litehtml/hello.html");


    hubbub_parser_parse_chunk(parser,utf8Html, 14752 ) == HUBBUB_OK;


    const char *charset;
    hubbub_charset_source cssource;
    hubbub_parser_read_charset(parser, &cssource);
    printf("Charset: %s (from %d)\n", charset, cssource);

    hubbub_parser_destroy(parser);

    printf("Hello, World!\n");
    return 0;
}


hubbub_error token_handler(const hubbub_token *token, void *pw)
{
    static const char *token_names[] = {
            "DOCTYPE", "START TAG", "END TAG",
            "COMMENT", "CHARACTERS", "EOF"
    };
    size_t i;


    printf("%s: ", token_names[token->type]);

    switch (token->type) {
        case HUBBUB_TOKEN_DOCTYPE:
            printf("'%.*s' %sids:\n",
                   (int) token->data.doctype.name.len,
                   token->data.doctype.name.ptr,
                   token->data.doctype.force_quirks ?
                   "(force-quirks) " : "");

            if (token->data.doctype.public_missing)
                printf("\tpublic: missing\n");
            else
                printf("\tpublic: '%.*s'\n",
                       (int) token->data.doctype.public_id.len,
                       token->data.doctype.public_id.ptr);

            if (token->data.doctype.system_missing)
                printf("\tsystem: missing\n");
            else
                printf("\tsystem: '%.*s'\n",
                       (int) token->data.doctype.system_id.len,
                       token->data.doctype.system_id.ptr);

            break;
        case HUBBUB_TOKEN_START_TAG:
            printf("'%.*s' %s%s\n",
                   (int) token->data.tag.name.len,
                   token->data.tag.name.ptr,
                   (token->data.tag.self_closing) ?
                   "(self-closing) " : "",
                   (token->data.tag.n_attributes > 0) ?
                   "attributes:" : "");
            for (i = 0; i < token->data.tag.n_attributes; i++) {
                printf("\t'%.*s' = '%.*s'\n",
                       (int) token->data.tag.attributes[i].name.len,
                       token->data.tag.attributes[i].name.ptr,
                       (int) token->data.tag.attributes[i].value.len,
                       token->data.tag.attributes[i].value.ptr);
            }
            break;
        case HUBBUB_TOKEN_END_TAG:
            printf("'%.*s' %s%s\n",
                   (int) token->data.tag.name.len,
                   token->data.tag.name.ptr,
                   (token->data.tag.self_closing) ?
                   "(self-closing) " : "",
                   (token->data.tag.n_attributes > 0) ?
                   "attributes:" : "");
            for (i = 0; i < token->data.tag.n_attributes; i++) {
                printf("\t'%.*s' = '%.*s'\n",
                       (int) token->data.tag.attributes[i].name.len,
                       token->data.tag.attributes[i].name.ptr,
                       (int) token->data.tag.attributes[i].value.len,
                       token->data.tag.attributes[i].value.ptr);
            }
            break;
        case HUBBUB_TOKEN_COMMENT:
            printf("'%.*s'\n", (int) token->data.comment.len,
                   token->data.comment.ptr);
            break;
        case HUBBUB_TOKEN_CHARACTER:
            printf("'%.*s'\n", (int) token->data.character.len,
                   token->data.character.ptr);
            break;
        case HUBBUB_TOKEN_EOF:
            printf("\n");
            break;
    }

    return HUBBUB_OK;
}
