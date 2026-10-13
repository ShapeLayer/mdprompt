#include "mdprompt.h"
#include <node.h>
#include <syntax_extension.h>
#include <parser.h>
#include <render.h>
#include <string.h>
#include <stdlib.h>

typedef struct { int instruction, block, closed, display; } meta;
static cmark_node_type inline_type;
static meta *info(cmark_node *n) {
  cmark_syntax_extension *e = cmark_node_get_syntax_extension(n);
  if (!e || strcmp(e->name, "mdprompt")) return NULL;
  return cmark_node_get_user_data(n);
}
static void release(cmark_mem *mem, void *p) { mem->free(p); }
static void annotate(cmark_node *n, cmark_syntax_extension *e, int instruction, int block, int closed) {
  cmark_mem *mem = n->content.mem;
  meta *m = mem->calloc(1, sizeof(*m));
  m->instruction=instruction; m->block=block; m->closed=closed;
  cmark_node_set_syntax_extension(n,e);
  cmark_node_set_user_data(n,m);
  cmark_node_set_user_data_free_func(n,release);
}
int mdprompt_is_instruction(cmark_node *n) { meta *m=info(n); return m && m->instruction; }
int mdprompt_is_closed(cmark_node *n) { meta *m=info(n); return !m || m->closed; }
const char *mdprompt_literal(cmark_node *n) {
  meta *m=info(n);
  if (!m) return NULL;
  return m->block ? cmark_node_get_literal(n) : (const char *)n->content.ptr;
}
static const char *type_name(cmark_syntax_extension *e,cmark_node *n) {
  meta *m=info(n);
  if (!m) return "mdprompt";
  return m->instruction ? (m->block ? "instruction_block" : "instruction_inline") : (m->block ? "math_block" : (m->display ? "math_display_inline" : "math_inline"));
}
static int contain(cmark_syntax_extension *e,cmark_node *n,cmark_node_type t) { return 0; }
static int inlines(cmark_syntax_extension *e,cmark_node *n) { return 0; }
static int escaped(const unsigned char *s,int pos,int begin) {
  int count=0; while(pos>begin && s[--pos]=='\\') count++; return count%2;
}
static cmark_node *match_inline(cmark_syntax_extension *e,cmark_parser *p,cmark_node *parent,unsigned char ch,cmark_inline_parser *ip) {
  cmark_chunk *s=cmark_inline_parser_get_chunk(ip);
  int start=cmark_inline_parser_get_offset(ip), width=ch=='^'?2:1;
  int instruction=ch=='^';
  if (instruction) {
    if(start+1>=s->len || s->data[start+1]!='%' || (start+2<s->len && s->data[start+2]=='%' && !(start+3<s->len && s->data[start+3]=='^'))) return NULL;
  } else {
    if(ch!='$' || (start>0 && s->data[start-1]=='$')) return NULL;
    if(start+1<s->len && s->data[start+1]=='$') width=2;
    if(start+width<s->len && s->data[start+width]=='$') return NULL;
  }
  int end=start+width;
  for(;end<s->len && s->data[end]!='\n' && s->data[end]!='\r';end++) {
    int closing=instruction ? (s->data[end]=='%' && end+1<s->len && s->data[end+1]=='^') : (s->data[end]=='$' && (width==1 ? (end+1>=s->len || s->data[end+1]!='$') : (end+1<s->len && s->data[end+1]=='$')));
    if(closing && !escaped(s->data,end,start+width)) break;
  }
  int closed=end<s->len && s->data[end]!='\n' && s->data[end]!='\r';
  if(!instruction && !closed) return NULL;
  cmark_node *n=cmark_node_new_with_mem(inline_type,p->mem);
  annotate(n,e,instruction,0,closed);
  info(n)->display=!instruction && width==2;
  cmark_strbuf_put(&n->content,s->data+start+width,end-start-width);
  n->start_line=cmark_inline_parser_get_line(ip);
  n->start_column=cmark_inline_parser_get_column(ip);
  n->end_line=n->start_line; n->end_column=n->start_column+end-start+(closed?width:0)-1;
  cmark_inline_parser_set_offset(ip,end+(closed?width:0));
  return n;
}
static int marker(unsigned char *s,int len,int offset,const char *mark) {
  int n=(int)strlen(mark);
  if(offset+n>len || memcmp(s+offset,mark,n)) return 0;
  for(int i=offset+n;i<len;i++) if(s[i]!=' ' && s[i]!='\t' && s[i]!='\r' && s[i]!='\n') return 0;
  return 1;
}
static cmark_node *open_block(cmark_syntax_extension *e,int indented,cmark_parser *p,cmark_node *parent,unsigned char *s,int len) {
  /* Version 0.1 block directives are root-level, at columns 1–4. */
  if(indented || cmark_node_get_type(parent)!=CMARK_NODE_DOCUMENT) return NULL;
  int offset=cmark_parser_get_first_nonspace(p);
  int instruction=marker(s,len,offset,"^%%");
  if(!instruction && !marker(s,len,offset,"$$")) return NULL;
  cmark_node *n=cmark_parser_add_child(p,parent,CMARK_NODE_CODE_BLOCK,offset+1);
  annotate(n,e,instruction,1,0);
  n->as.code.fenced=1; n->as.code.fence_length=0;
  n->as.code.info=cmark_chunk_literal("");
  cmark_parser_advance_offset(p,(const char *)s,len-cmark_parser_get_offset(p)-1,0);
  return n;
}
static int match_block(cmark_syntax_extension *e,cmark_parser *p,unsigned char *s,int len,cmark_node *n) {
  meta *m=info(n);
  if(m->closed) return 0;
  int offset=cmark_parser_get_first_nonspace(p);
  if(cmark_parser_get_indent(p)<4 && marker(s,len,offset,m->instruction?"%%^":"$$")) {
    m->closed=1;
    cmark_parser_advance_offset(p,(const char *)s,len-cmark_parser_get_offset(p)-1,0);
  }
  return 1;
}
static cmark_node *postprocess(cmark_syntax_extension *e,cmark_parser *p,cmark_node *root) {
  cmark_iter *it=cmark_iter_new(root); cmark_event_type ev;
  while((ev=cmark_iter_next(it))!=CMARK_EVENT_DONE) {
    cmark_node *n=cmark_iter_get_node(it); meta *m=info(n);
    if(ev==CMARK_EVENT_ENTER && m && m->block && m->closed) {
      cmark_chunk *s=&n->as.code.literal;
      if(s->len && s->data[s->len-1]=='\n') { s->len--; s->data[s->len]=0; }
    }
  }
  cmark_iter_free(it); return root;
}
static void escape_html(cmark_strbuf *b,const char *s) {
  for(;*s;s++) {
    switch(*s) {
      case '&': cmark_strbuf_puts(b,"&amp;");break;
      case '<': cmark_strbuf_puts(b,"&lt;");break;
      case '>': cmark_strbuf_puts(b,"&gt;");break;
      case '"': cmark_strbuf_puts(b,"&quot;");break;
      default: cmark_strbuf_putc(b,*s);
    }
  }
}
static void html(cmark_syntax_extension *e,cmark_html_renderer *r,cmark_node *n,cmark_event_type ev,int options) {
  meta *m=info(n); if(!m || m->instruction || ev!=CMARK_EVENT_ENTER) return;
  cmark_strbuf_puts(r->html,m->block?"<div class=\"math display\">\\[":(m->display?"<span class=\"math display\">\\[":"<span class=\"math inline\">\\("));
  escape_html(r->html,mdprompt_literal(n));
  cmark_strbuf_puts(r->html,m->block?"\\]</div>\n":(m->display?"\\]</span>":"\\)</span>"));
}
static void markdown(cmark_syntax_extension *e,cmark_renderer *r,cmark_node *n,cmark_event_type ev,int options) {
  meta *m=info(n); if(!m || m->instruction || ev!=CMARK_EVENT_ENTER) return;
  if(m->block) r->blankline(r);
  r->out(r,n,m->block?"$$\n":(m->display?"$$":"$"),0,LITERAL);
  r->out(r,n,mdprompt_literal(n),0,LITERAL);
  r->out(r,n,(m->block || m->display)?"$$":"$",0,LITERAL);
  if(m->block) r->blankline(r);
}
cmark_syntax_extension *mdprompt_create_extension(void) {
  cmark_syntax_extension *e=cmark_syntax_extension_new("mdprompt");
  if(!inline_type) inline_type=cmark_syntax_extension_add_node(1);
  cmark_syntax_extension_set_get_type_string_func(e,type_name);
  cmark_syntax_extension_set_can_contain_func(e,contain);
  cmark_syntax_extension_set_contains_inlines_func(e,inlines);
  cmark_syntax_extension_set_match_inline_func(e,match_inline);
  cmark_syntax_extension_set_open_block_func(e,open_block);
  cmark_syntax_extension_set_match_block_func(e,match_block);
  cmark_syntax_extension_set_postprocess_func(e,postprocess);
  cmark_syntax_extension_set_html_render_func(e,html);
  cmark_syntax_extension_set_commonmark_render_func(e,markdown);
  cmark_syntax_extension_set_plaintext_render_func(e,markdown);
  cmark_llist *chars=NULL; cmark_mem *mem=cmark_get_default_mem_allocator();
  chars=cmark_llist_append(mem,chars,(void *)'^');
  chars=cmark_llist_append(mem,chars,(void *)'$');
  cmark_syntax_extension_set_special_inline_chars(e,chars);
  return e;
}
int init_mdprompt(cmark_plugin *plugin) { return cmark_plugin_register_syntax_extension(plugin,mdprompt_create_extension()); }
