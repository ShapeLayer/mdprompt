#ifndef MDPROMPT_H
#define MDPROMPT_H
#include <cmark-gfm-extension_api.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Attach this extension before feeding the parser. Keep it alive until AST freed. */
cmark_syntax_extension *mdprompt_create_extension(void);
int init_mdprompt(cmark_plugin *plugin);
int mdprompt_is_instruction(cmark_node *node);
int mdprompt_is_closed(cmark_node *node);
const char *mdprompt_literal(cmark_node *node);
#ifdef __cplusplus
}
#endif
#endif
