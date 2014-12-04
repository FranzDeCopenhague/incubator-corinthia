// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "TestFunctions.h"
#include "DFBDT.h"
#include "BDTTest.h"
#include "Plain.h"
#include "Commands.h"
#include "DFChanges.h"
#include "WordConverter.h"
#include "DFHTML.h"
#include "DFHTMLNormalization.h"
#include "DFFilesystem.h"
#include "DFString.h"
#include "HTMLToLaTeX.h"
#include "DFXML.h"
#include "DFCommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*TestFunction)(TestCase *script, int argc, const char **argv);

static void move(TestCase *script, int argc, const char **argv)
{
    if (argc < 3) {
        DFBufferFormat(script->output,"move: insufficient arguments");
        return;
    }

    int count = atoi(argv[0]);
    int from = atoi(argv[1]);
    int to = atoi(argv[2]);

    DFBuffer *output = DFBufferNew();
    BDT_testMove(count,from,to,output);
    DFBufferFormat(script->output,"%s",output->data);
    DFBufferRelease(output);
}

static void removeChildren(TestCase *script, int argc, const char **argv)
{
    int *indices = (int *)malloc(argc*sizeof(int));

    for (int i = 0; i < argc; i++) {
        int index = atoi(argv[i]);
        indices[i] = index;
    }

    DFBuffer *output = DFBufferNew();
    BDT_testRemove(indices,argc,output);
    DFBufferFormat(script->output,"%s",output->data);
    DFBufferRelease(output);

    free(indices);
}

static void CSS_setHeadingNumbering(TestCase *script, int argc, const char **argv)
{
    const char *inputCSS = DFHashTableLookup(script->input,"input.css");
    if (inputCSS == NULL) {
        DFBufferFormat(script->output,"CSS_setHeadingNumbering: input.css not defined");
        return;
    }
    if (argc < 1) {
        DFBufferFormat(script->output,"CSS_setHeadingNumbering: expected 1 argument");
        return;
    }

    CSSSheet *styleSheet = CSSSheetNew();
    CSSSheetUpdateFromCSSText(styleSheet,inputCSS);
    int on = !strcasecmp(argv[0],"true");
    CSSSheetSetHeadingNumbering(styleSheet,on);
    char *cssText = CSSSheetCopyCSSText(styleSheet);
    DFBufferFormat(script->output,"%s",cssText);
    free(cssText);
    CSSSheetRelease(styleSheet);
}

static void Word_testCollapseBookmarks(TestCase *script, int argc, const char **argv)
{
    DFError *error = NULL;
    WordPackage *wordPackage = NULL;
    DFPackage *rawPackage = NULL;
    if (!TestCaseOpenWordPackage(script,&wordPackage,&rawPackage,&error)) { // Logs error itself on failure
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    WordPackageCollapseBookmarks(wordPackage);

    DFHashTable *parts = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(parts,"document","");

    // Output the docx file
    char *plain = Word_toPlain(wordPackage,rawPackage,parts);
    DFBufferFormat(script->output,"%s",plain);
    free(plain);
    DFHashTableRelease(parts);
    WordPackageRelease(wordPackage);
    DFPackageRelease(rawPackage);
}


static void Word_testExpandBookmarks(TestCase *script, int argc, const char **argv)
{
    DFError *error = NULL;
    WordPackage *wordPackage = NULL;
    DFPackage *rawPackage = NULL;
    if (!TestCaseOpenWordPackage(script,&wordPackage,&rawPackage,&error)) { // Logs error itself on failure
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    WordPackageExpandBookmarks(wordPackage);

    DFHashTable *parts = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(parts,"document","");

    // Output the docx file
    char *plain = Word_toPlain(wordPackage,rawPackage,parts);
    DFBufferFormat(script->output,"%s",plain);
    free(plain);
    DFHashTableRelease(parts);
    WordPackageRelease(wordPackage);
    DFPackageRelease(rawPackage);
}

static void Word_testGet2(TestCase *script, WordPackage *package, int argc, const char **argv)
{
    DFError *error = NULL;
    // Create the HTML file
    // FIXME: maybe use a temporary directory for the image path?
    DFPackage *abstractPackage = DFPackageNewFilesystem(script->abstractPath,DFFileFormatHTML);
    DFDocument *htmlDoc = WordPackageGenerateHTML(package,abstractPackage,"word",&error,NULL);
    DFPackageRelease(abstractPackage);
    if (htmlDoc == NULL) {
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    // Output the HTML file
    HTML_safeIndent(htmlDoc->docNode,0);

    char *htmlPlain = HTML_toPlain(htmlDoc,script->abstractPath,&error);
    DFDocumentRelease(htmlDoc);

    if (htmlPlain == NULL) {
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }
    DFBufferFormat(script->output,"%s",htmlPlain);
    free(htmlPlain);
}

static void Word_testGet(TestCase *script, int argc, const char **argv)
{
    DFError *error = NULL;
    WordPackage *wordPackage = NULL;
    DFPackage *rawPackage = NULL;
    if (!TestCaseOpenWordPackage(script,&wordPackage,&rawPackage,&error)) { // Logs error itself on failure
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    Word_testGet2(script,wordPackage,argc,argv);
    WordPackageRelease(wordPackage);
    DFPackageRelease(rawPackage);
}

static DFHashTable *getFlags(int argc, const char **argv)
{
    if (argc == 0)
        return NULL;;
    DFHashTable *set = DFHashTableNew((DFCopyFunction)strdup,free);
    for (int i = 0; i < argc; i++) {
        const char *colon = strchr(argv[i],':');
        if (colon == NULL)
            continue;
        size_t colonPos = colon - argv[i];
        char *rawName = DFSubstring(argv[i],0,colonPos);
        char *rawValue = DFSubstring(argv[i],colonPos+1,strlen(argv[i]));
        char *name = DFStringTrimWhitespace(rawName);
        char *value = DFStringTrimWhitespace(rawValue);
        if (!strcasecmp(value,"true"))
            DFHashTableAdd(set,name,name);

        free(rawName);
        free(rawValue);
        free(name);
        free(value);
    }
    return set;
}

static void Word_testCreate(TestCase *script, int argc, const char **argv)
{
    DFPackage *rawPackage = DFPackageNewMemory(DFFileFormatDocx);
    WordPackage *wordPackage = NULL;

    DFDocument *htmlDoc = NULL;
    DFHashTable *parts = NULL;
    char *plain = NULL;
    DFError *error = NULL;
    DFPackage *abstractPackage = NULL;

    // Read input.html
    htmlDoc = TestCaseGetHTML(script);
    if (htmlDoc == NULL)
        goto end;;

    // Create the docx file
    wordPackage = WordPackageOpenNew(rawPackage,&error);
    if (wordPackage == NULL) {
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        goto end;
    }

    DFBuffer *warnings = DFBufferNew();
    abstractPackage = DFPackageNewFilesystem(script->abstractPath,DFFileFormatHTML);
    if (!WordPackageUpdateFromHTML(wordPackage,htmlDoc,abstractPackage,"word",&error,warnings)) {
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        goto end;
    }

    if (warnings->len > 0) {
        DFBufferFormat(script->output,"%s",warnings->data);
        DFBufferRelease(warnings);
        goto end;
    }
    DFBufferRelease(warnings);

    // We don't actually "save" the package as such; this is just to ensure the missing OPC parts are added
    WordPackageSave(wordPackage,NULL);

    // Output the docx file
    parts = getFlags(argc,argv);
    plain = Word_toPlain(wordPackage,rawPackage,parts);
    DFBufferFormat(script->output,"%s",plain);

end:
    DFDocumentRelease(htmlDoc);
    free(plain);
    DFHashTableRelease(parts);
    DFErrorRelease(error);
    DFPackageRelease(rawPackage);
    WordPackageRelease(wordPackage);
    DFPackageRelease(abstractPackage);
}

static void Word_testUpdate2(TestCase *script, WordPackage *wordPackage, DFPackage *rawPackage, int argc, const char **argv)
{
    // Read input.html
    DFDocument *htmlDoc = TestCaseGetHTML(script);
    if (htmlDoc == NULL)
        return;;

    // Update the docx file based on the contents of the HTML file
    DFError *error = NULL;
    DFPackage *abstractPackage = DFPackageNewFilesystem(script->abstractPath,DFFileFormatHTML);
    if (!WordPackageUpdateFromHTML(wordPackage,htmlDoc,abstractPackage,"word",&error,NULL)) {
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        DFPackageRelease(abstractPackage);
        return;
    }
    DFPackageRelease(abstractPackage);

    // We don't actually "save" the package as such; this is just to ensure the missing OPC parts are added
    WordPackageSave(wordPackage,NULL);

    // Output the updated docx file
    DFHashTable *parts = getFlags(argc,argv);
    char *plain = Word_toPlain(wordPackage,rawPackage,parts);
    DFBufferFormat(script->output,"%s",plain);
    free(plain);
    DFHashTableRelease(parts);
    DFDocumentRelease(htmlDoc);
}

static void Word_testUpdate(TestCase *script, int argc, const char **argv)
{
    DFError *error = NULL;
    WordPackage *wordPackage = NULL;
    DFPackage *rawPackage = NULL;
    if (!TestCaseOpenWordPackage(script,&wordPackage,&rawPackage,&error)) { // Logs error itself on failure
        DFBufferFormat(script->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    Word_testUpdate2(script,wordPackage,rawPackage,argc,argv);
    WordPackageRelease(wordPackage);
    DFPackageRelease(rawPackage);
}

static void LaTeX_testCreate(TestCase *script, int argc, const char **argv)
{
    DFDocument *htmlDoc = TestCaseGetHTML(script);
    if (htmlDoc == NULL)
        return;

    HTML_normalizeDocument(htmlDoc);
    char *latex = HTMLToLaTeX(htmlDoc);
    DFBufferFormat(script->output,"%s",latex);
    free(latex);
    DFDocumentRelease(htmlDoc);
}

static void HTML_testNormalize(TestCase *script, int argc, const char **argv)
{
    const char *inputHtml = DFHashTableLookup(script->input,"input.html");
    if (inputHtml == NULL) {
        DFBufferFormat(script->output,"input.html not defined");
        return;
    }
    DFError *error = NULL;
    DFDocument *doc = DFParseHTMLString(inputHtml,0,&error);
    if (doc == NULL) {
        DFBufferFormat(script->output,"%s",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }
    HTML_normalizeDocument(doc);
    HTML_safeIndent(doc->docNode,0);
    char *docStr = DFSerializeXMLString(doc,0,0);
    DFBufferFormat(script->output,"%s",docStr);
    free(docStr);
    DFDocumentRelease(doc);
}

static void HTML_showChanges(TestCase *script, int argc, const char **argv)
{
    const char *input1 = DFHashTableLookup(script->input,"input1.html");
    if (input1 == NULL) {
        DFBufferFormat(script->output,"input.html not defined");
        return;
    }

    const char *input2 = DFHashTableLookup(script->input,"input2.html");
    if (input2 == NULL) {
        DFBufferFormat(script->output,"input.html not defined");
        return;
    }


    DFError *error = NULL;
    DFDocument *doc1 = DFParseHTMLString(input1,0,&error);
    if (doc1 == NULL) {
        DFBufferFormat(script->output,"%s",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    DFDocument *doc2 = DFParseHTMLString(input2,0,&error);
    if (doc2 == NULL) {
        DFBufferFormat(script->output,"%s",DFErrorMessage(&error));
        DFErrorRelease(error);
        DFDocumentRelease(doc1);
    }

    DFComputeChanges(doc1->root,doc2->root,HTML_ID);

    char *changesStr = DFChangesToString(doc1->root);
    DFBufferFormat(script->output,"%s",changesStr);
    free(changesStr);
    DFDocumentRelease(doc1);
    DFDocumentRelease(doc2);
}

static void CSS_test(TestCase *script, int argc, const char **argv)
{
    const char *inputCSS = DFHashTableLookup(script->input,"input.css");
    if (inputCSS == NULL) {
        DFBufferFormat(script->output,"input.css not defined");
        return;
    }
    CSSSheet *styleSheet = CSSSheetNew();
    CSSSheetUpdateFromCSSText(styleSheet,inputCSS);
    char *text = CSSSheetCopyText(styleSheet);
    DFBufferFormat(script->output,"%s",text);
    free(text);
    DFBufferFormat(script->output,
        "================================================================================\n");
    char *cssText = CSSSheetCopyCSSText(styleSheet);
    DFBufferFormat(script->output,"%s",cssText);
    free(cssText);
    CSSSheetRelease(styleSheet);
}

static struct {
    const char *name;
    TestFunction fun;
} testFunctions[] = {
    { "CSS_setHeadingNumbering", CSS_setHeadingNumbering },
    { "Word_testCollapseBookmarks", Word_testCollapseBookmarks },
    { "Word_testExpandBookmarks", Word_testExpandBookmarks },
    { "Word_testGet", Word_testGet },
    { "Word_testCreate", Word_testCreate },
    { "Word_testUpdate", Word_testUpdate },
    { "LaTeX_testCreate", LaTeX_testCreate },
    { "HTML_testNormalize", HTML_testNormalize },
    { "HTML_showChanges", HTML_showChanges },
    { "CSS_test", CSS_test },
    { "move", move },
    { "remove", removeChildren },
    { NULL, NULL },
};

static int testSetup(TestCase *script)
{
    DFError *error = NULL;
    script->tempPath = createTempDir(&error);
    if (script->tempPath == NULL) {
        DFBufferFormat(script->output,"createTempDir: %s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return 0;
    }

    script->abstractPath = DFAppendPathComponent(script->tempPath,"abstract");

    if (!DFCreateDirectory(script->abstractPath,1,&error)) {
        DFBufferFormat(script->output,"%s: %s",script->abstractPath,DFErrorMessage(&error));
        DFErrorRelease(error);
        return 0;
    }

    return 1;
}

static void testTeardown(TestCase *script)
{
    DFDeleteFile(script->tempPath,NULL);
}

void runTest(TestCase *script, const char *name, int argc, const char **argv)
{
    for (int i = 0; testFunctions[i].name != NULL; i++) {
        if (!strcmp(testFunctions[i].name,name)) {
            if (testSetup(script)) {
                testFunctions[i].fun(script,argc,argv);
                testTeardown(script);
            }
            return;
        }
    }
    printf("runTest %s\n",name);
    for (int i = 0; i < argc; i++) {
        printf("    argv[%d] = %s\n",i,argv[i]);
    }
    DFBufferFormat(script->output,"Unknown test: %s\n",name);
}
