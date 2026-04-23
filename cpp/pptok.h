/*
 * pptok.h — Preprocessing tokenizer interface.
 */
#ifndef SHARP_PPTOK_H
#define SHARP_PPTOK_H

#include "cpp_internal.h"

/** Create a reader from a file on disk. Returns NULL on I/O error. */
CppReader *reader_new_from_file(const char *filename,
                                InternTable *interns,
                                CppDiagArr *diags);

/** Create a reader from an in-memory buffer (not copied; must outlive reader). */
CppReader *reader_new_from_buf(const char *buf, size_t len,
                               const char *filename,
                               InternTable *interns,
                               CppDiagArr *diags);

/** Free a reader. */
void reader_free(CppReader *rd);

/** Return the interned filename string associated with this reader. */
const char *reader_filename(const CppReader *rd);

/** Return the current source line number of this reader (1-based). */
int reader_current_line(const CppReader *rd);

/** Set the reader's line number (for #line directive). */
void reader_set_line(CppReader *rd, int line);

/** Return the reader's column number (1-based). */
int reader_current_col(const CppReader *rd);

/**
 * Produce the next preprocessing token.
 *
 * `keep_comments` — when true, block/line comments are returned as
 *    CPPT_COMMENT tokens with their full text.  When false they are
 *    collapsed to a single CPPT_SPACE token (block) or discarded (line).
 *
 * Whitespace is returned as CPPT_SPACE.  Newlines (logical, after phase 2)
 * are returned as CPPT_NEWLINE.  Both may be discarded by callers that do
 * not need them for output formatting.
 *
 * Returns a PPTok whose `spell.buf` is heap-allocated; caller must eventually
 * call pptok_free().
 */
PPTok reader_next_tok(CppReader *rd, bool keep_comments);

#endif /* SHARP_PPTOK_H */
