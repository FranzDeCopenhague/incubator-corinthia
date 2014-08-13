//
//  ODFManifest.c
//  DocFormats
//
//  Created by Peter Kelly on 20/12/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#include "ODFManifest.h"
#include "DFXMLNames.h"
#include "DFCommon.h"

static void ODFManifestParse(ODFManifest *manifest)
{
    for (DFNode *child = manifest->doc->root; child != NULL; child = child->next) {
        if (child->tag == MF_FILE_ENTRY) {
            const char *path = DFGetAttribute(child,MF_FULL_PATH);
            if (path != NULL)
                DFHashTableAdd(manifest->entriesByPath,path,child);
        }
    }
}

ODFManifest *ODFManifestNew(void)
{
    ODFManifest *manifest = (ODFManifest *)calloc(1,sizeof(ODFManifest));
    manifest->retainCount = 1;
    manifest->doc = DFDocumentNewWithRoot(MF_MANIFEST);
    manifest->entriesByPath = DFHashTableNew(NULL,NULL);
    return manifest;
}

ODFManifest *ODFManifestNewWithDoc(DFDocument *doc)
{
    ODFManifest *manifest = (ODFManifest *)calloc(1,sizeof(ODFManifest));
    manifest->doc = DFDocumentRetain(doc);
    manifest->entriesByPath = DFHashTableNew(NULL,NULL);
    ODFManifestParse(manifest);
    return manifest;
}

ODFManifest *ODFManifestRetain(ODFManifest *manifest)
{
    if (manifest != NULL)
        manifest->retainCount++;
    return manifest;
}

void ODFManifestRelease(ODFManifest *manifest)
{
    if ((manifest == NULL) || (--manifest->retainCount > 0))
        return;

    DFDocumentRelease(manifest->doc);
    DFHashTableRelease(manifest->entriesByPath);
    free(manifest);
}

void ODFManifestAddEntry(ODFManifest *manifest, const char *path, const char *mediaType,
                         const char *version)
{
    DFNode *entry = DFHashTableLookup(manifest->entriesByPath,path);
    if (entry == NULL) {
        entry = DFCreateChildElement(manifest->doc->root,MF_FILE_ENTRY);
        DFHashTableAdd(manifest->entriesByPath,path,entry);
    }
    DFSetAttribute(entry,MF_FULL_PATH,path);
    DFSetAttribute(entry,MF_MEDIA_TYPE,mediaType);
    DFSetAttribute(entry,MF_VERSION,version);
    if (!strcmp(path,"") && (version != NULL))
        DFSetAttribute(manifest->doc->root,MF_VERSION,version);
}
