#include "mdprompt.h"
#include <cmark-gfm-core-extensions.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void json(const char *s) {
  putchar('"');
  for(const unsigned char *p=(const unsigned char *)s;*p;p++) {
    switch(*p) {
      case '"': fputs("\\\"",stdout);break;
      case '\\': fputs("\\\\",stdout);break;
      case '\n': fputs("\\n",stdout);break;
      case '\r': fputs("\\r",stdout);break;
      case '\t': fputs("\\t",stdout);break;
      default: if(*p<32) printf("\\u%04x",*p); else putchar(*p);
    }
  } putchar('"');
}
int main(int argc,char **argv) {
  const char *mode=argc>1?argv[1]:"--json";
  if(argc>2 || (strcmp(mode,"--json") && strcmp(mode,"--html") && strcmp(mode,"--content"))) {
    fputs("usage: mdprompt-cli [--json|--html|--content] < document.mdp\n",stderr);return 2;
  }
  cmark_gfm_core_extensions_ensure_registered();
  cmark_parser *p=cmark_parser_new(CMARK_OPT_DEFAULT);
  const char *names[]={"table","strikethrough","autolink","tagfilter","tasklist"};
  for(size_t i=0;i<sizeof(names)/sizeof(*names);i++) {
    cmark_syntax_extension *e=cmark_find_syntax_extension(names[i]);
    if(!e || !cmark_parser_attach_syntax_extension(p,e)) { fputs("GFM extension unavailable\n",stderr);return 2; }
  }
  cmark_syntax_extension *ext=mdprompt_create_extension();
  cmark_parser_attach_syntax_extension(p,ext);
  char buf[8192];size_t count;
  while((count=fread(buf,1,sizeof(buf),stdin))) cmark_parser_feed(p,buf,count);
  if(ferror(stdin)) { fputs("input error\n",stderr);return 2; }
  cmark_node *root=cmark_parser_finish(p);
  cmark_iter *it=cmark_iter_new(root);cmark_event_type ev; int invalid=0;
  while((ev=cmark_iter_next(it))!=CMARK_EVENT_DONE) {
    cmark_node *n=cmark_iter_get_node(it);
    if(ev==CMARK_EVENT_ENTER && !mdprompt_is_closed(n)) {
      fprintf(stderr,"unclosed mdprompt region at %d:%d\n",cmark_node_get_start_line(n),cmark_node_get_start_column(n)); invalid=1;
    }
  }
  cmark_iter_free(it);
  if(!invalid) {
    char *content=cmark_render_commonmark(root,0,0);
    if(!strcmp(content,"\n")) content[0]=0;
    if(!strcmp(mode,"--content")) fputs(content,stdout);
    else if(!strcmp(mode,"--html")) {
      char *html=cmark_render_html(root,0,cmark_parser_get_syntax_extensions(p)); fputs(html,stdout); free(html);
    } else {
      fputs("{\"format\":\"mdprompt\",\"version\":\"0.1\",\"content\":",stdout);json(content);
      fputs(",\"instructions\":[",stdout);int first=1;
      it=cmark_iter_new(root);
      while((ev=cmark_iter_next(it))!=CMARK_EVENT_DONE) {
        cmark_node *n=cmark_iter_get_node(it);
        if(ev==CMARK_EVENT_ENTER && mdprompt_is_instruction(n)) {
          if(!first) putchar(',');first=0;
          fputs("{\"text\":",stdout);json(mdprompt_literal(n));
          printf(",\"line\":%d,\"column\":%d,\"kind\":",cmark_node_get_start_line(n),cmark_node_get_start_column(n));
          json(cmark_node_get_type_string(n));
          if(cmark_node_get_type(n)==CMARK_NODE_CODE_BLOCK) {
            cmark_node *before=cmark_node_previous(n), *after=cmark_node_next(n);
            char *context=before?cmark_render_commonmark(before,0,0):NULL;
            fputs(",\"context_before\":",stdout);json(context?context:"");free(context);
            context=after?cmark_render_commonmark(after,0,0):NULL;
            fputs(",\"context_after\":",stdout);json(context?context:"");free(context);
          } else {
            char *context=cmark_render_commonmark(cmark_node_parent(n),0,0);
            fputs(",\"context\":",stdout);json(context);free(context);
          }
          putchar('}');
        }
      }
      cmark_iter_free(it);fputs("]}\n",stdout);
    }
    free(content);
  }
  cmark_node_free(root);cmark_parser_free(p);
  cmark_syntax_extension_free(cmark_get_default_mem_allocator(),ext);
  return invalid?1:0;
}
