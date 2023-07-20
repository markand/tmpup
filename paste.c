/*
 * paste.c -- paste definition
 *
 * Copyright (c) 2023 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "paste.h"
#include "tmp.h"
#include "util.h"

const char * const paste_langs[] = {
	"1c",
	"abnf",
	"accesslog",
	"actionscript",
	"ada",
	"apache",
	"applescript",
	"arduino",
	"armasm",
	"asciidoc",
	"aspectj",
	"autohotkey",
	"autoit",
	"avrasm",
	"awk",
	"axapta",
	"bash",
	"basic",
	"bnf",
	"brainfuck",
	"cal",
	"capnproto",
	"ceylon",
	"clean",
	"clojure",
	"clojure-repl",
	"cmake",
	"coffeescript",
	"coq",
	"cos",
	"cpp",
	"crmsh",
	"crystal",
	"cs",
	"csp",
	"css",
	"d",
	"dart",
	"delphi",
	"diff",
	"django",
	"dns",
	"dockerfile",
	"dos",
	"dsconfig",
	"dts",
	"dust",
	"ebnf",
	"elixir",
	"elm",
	"erb",
	"erlang",
	"erlang-repl",
	"excel",
	"fix",
	"flix",
	"fortran",
	"fsharp",
	"gams",
	"gauss",
	"gcode",
	"gherkin",
	"glsl",
	"go",
	"golo",
	"gradle",
	"groovy",
	"haml",
	"handlebars",
	"haskell",
	"haxe",
	"hsp",
	"htmlbars",
	"http",
	"hy",
	"inform7",
	"ini",
	"irpf90",
	"java",
	"javascript",
	"jboss-cli",
	"json",
	"julia",
	"julia-repl",
	"kotlin",
	"lasso",
	"ldif",
	"leaf",
	"less",
	"lisp",
	"livecodeserver",
	"livescript",
	"llvm",
	"lsl",
	"lua",
	"makefile",
	"markdown",
	"mathematica",
	"matlab",
	"maxima",
	"mel",
	"mercury",
	"mipsasm",
	"mizar",
	"mojolicious",
	"monkey",
	"moonscript",
	"n1ql",
	"nginx",
	"nimrod",
	"nix",
	"nohighlight",
	"nsis",
	"objectivec",
	"ocaml",
	"openscad",
	"oxygene",
	"parser3",
	"perl",
	"pf",
	"php",
	"pony",
	"powershell",
	"processing",
	"profile",
	"prolog",
	"protobuf",
	"puppet",
	"purebasic",
	"python",
	"q",
	"qml",
	"r",
	"rib",
	"roboconf",
	"routeros",
	"rsl",
	"ruby",
	"ruleslanguage",
	"rust",
	"scala",
	"scheme",
	"scilab",
	"scss",
	"shell",
	"smali",
	"smalltalk",
	"sml",
	"sqf",
	"sql",
	"stan",
	"stata",
	"step21",
	"stylus",
	"subunit",
	"swift",
	"taggerscript",
	"tap",
	"tcl",
	"tex",
	"thrift",
	"tp",
	"twig",
	"typescript",
	"vala",
	"vbnet",
	"vbscript",
	"vbscript-html",
	"verilog",
	"vhdl",
	"vim",
	"x86asm",
	"xl",
	"xml",
	"xquery",
	"yaml",
	"zephir"
};

const size_t paste_langsz = LEN(paste_langs);

void
paste_init(struct paste *paste,
           const char *id,
           const char *title,
           const char *author,
           const char *filename,
           const char *language,
           const char *code,
           time_t start,
           time_t end)
{
	assert(paste);
	assert(code);

	memset(paste, 0, sizeof (*paste));

	if (id)
		paste->id = estrdup(id);
	else
		paste->id = tmp_id();

	paste->title = estrdup(title ? title : TMP_DEFAULT_TITLE);
	paste->author = estrdup(author ? author : TMP_DEFAULT_AUTHOR);
	paste->filename = estrdup(filename ? filename : TMP_DEFAULT_FILENAME);
	paste->language = estrdup(language ? language : TMP_DEFAULT_LANG);
	paste->code = estrdup(code);
	paste->start = start;
	paste->end = end;
}

void
paste_finish(struct paste *paste)
{
	assert(paste);

	free(paste->id);
	free(paste->title);
	free(paste->author);
	free(paste->language);
	free(paste->code);
	memset(paste, 0, sizeof (*paste));
}

char *
paste_dump(const struct paste *paste)
{
	assert(paste);

	return tmp_json("{ss* ss* ss* ss* ss* ss sI sI}",
		"id",           paste->id,
		"title",        paste->title,
		"author",       paste->author,
		"filename",     paste->filename,
		"language",     paste->language,
		"code",         paste->code,
		"start",        (json_int_t)paste->start,
		"end",          (json_int_t)paste->end
	);
}

int
paste_parse(struct paste *paste, const char *text, char *error, size_t errorsz)
{
	const char *title = NULL, *author = NULL, *filename = NULL, *language = NULL, *code = NULL;
	json_int_t start = 0, end = 0;
	json_t *doc;
	json_error_t err;
	int rv;

	memset(paste, 0, sizeof (*paste));

	rv = tmp_parse(&doc, &err, text, "{s?s s?s s?s s?s s?I s?I}",
		"title",        &title,
		"author",       &author,
		"language",     &language,
		"code",         &code,
		"start",        &start,
		"end",          &end
	);

	if (rv < 0) {
		bstrlcpy(error, err.text, errorsz);
		return -1;
	}

	/* Check language exists. */
	if (check_language(language, error, errorsz) < 0 ||
	    check_duration(start, end, error, errorsz) < 0)
		return -1;

	paste_init(paste, NULL, title, author, filename, language, code, start, end);
	json_decref(doc);

	return 0;
}
