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

#ifndef dfutil_Plain_h
#define dfutil_Plain_h

#include <DocFormats/DFError.h>
#include "DFHashTable.h"
#include "DFDOM.h"
#include <DocFormats/DFStorage.h>

char *Word_toPlain(DFStorage *rawStorage, DFHashTable *parts);
DFStorage *Word_fromPlain(const char *plain, const char *plainPath, DFError **error);
char *HTML_toPlain(DFDocument *doc, DFStorage *storage, DFError **error);
DFDocument *HTML_fromPlain(const char *plain, const char *path, DFStorage *htmlStorage, DFError **error);

#endif
