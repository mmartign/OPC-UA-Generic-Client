/*
 * This work has been written on the 15th November 2019 by M. Martignano
 * affiliated to Spazio IT - Soluzioni Informatiche s.a.s.
 * See https://www.spazioit.com for more information.
 *
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See https://creativecommons.org/publicdomain/zero/1.0/ for more information.
 *
 */

#include <open62541/client_config.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>

#include <stdlib.h>

#ifdef UA_ENABLE_SUBSCRIPTIONS
static void
handler_TheAnswerChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
                         UA_UInt32 monId, void *monContext, UA_DataValue *value)
{
    printf("The Answer has changed!\n");
}
#endif

static UA_StatusCode
nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle)
{
    if (isInverse) {
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId *parent = (UA_NodeId *)handle;
    printf("%d, %d --- %d ---> NodeId %d, %d\n",
           parent->namespaceIndex, parent->identifier.numeric,
           referenceTypeId.identifier.numeric, childId.namespaceIndex,
           childId.identifier.numeric);
    return UA_STATUSCODE_GOOD;
}

#define MAX_LEN 255
char filename[MAX_LEN + 1];
FILE *fw = (FILE *)NULL;

#define OUT_FILE "C:\\mantovanelle\\silos\\getvar.out"

int main(int argc, char *argv[])
{
    int result = 0;

    if (argc != 5) {
        fprintf(stderr, "%s: wrong command line.\n", argv[0]);
        fprintf(stderr, "syntax: %s <OPC-UA Server URL> <Name Space Index> <Node Id> <Type>\n", argv[0]);
        return -1;
    }
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    strncpy(filename, OUT_FILE, MAX_LEN);

    if ((fw = fopen(filename, "w")) == (FILE *) NULL) {
        fprintf(stderr, "%s: cannot open output file %s.\n", argv[0], filename);
        return -2;
    }

    /* Connect to a server */
    /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
    // UA_StatusCode retval = UA_Client_connect_username(client, "opc.tcp://localhost:4840", "user", "password");
    UA_StatusCode retval = UA_Client_connect(client, argv[1]);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        fprintf(stderr, "%s: cannot connect to %s.\n", argv[0], argv[1]);
        fprintf(fw, "XXX-NO-VALUE\n");
        fclose(fw);
        return -3;
    }

    int ns = 0;
    UA_Boolean valueBoolean = 0;
    UA_Int16 valueInt16 = 0;
    UA_Int32 valueInt32 = 0;
    UA_Int64 valueInt64 = 0;
    UA_UInt16 valueUInt16 = 0;
    UA_UInt32 valueUInt32 = 0;
    UA_UInt64 valueUInt64 = 0;
    UA_Float valueFloat = 0.0;
    UA_Double valueDouble = 0.0;
    UA_String *ptrValueString;
    if ((sscanf(argv[2], "%d", &ns) != 1) ||
        (ns < 0)) {
        fprintf(stderr, "%s: <Name Space Index> must be a positive integer and not %s.\n", argv[0], argv[2]);
        fprintf(fw, "XXX-NO-VALUE\n");
        fclose(fw);
        return -4;
    }

    const UA_DataType *ptype = (UA_DataType *)NULL;
    /* Read attribute */
    if (strcmp(argv[4], "BOOL") == 0) {
        ptype = &UA_TYPES[UA_TYPES_BOOLEAN];
    }
    else if (strcmp(argv[4], "INT16") == 0) {
        ptype = &UA_TYPES[UA_TYPES_INT16];
    }
    else if (strcmp(argv[4], "INT32") == 0) {
        ptype = &UA_TYPES[UA_TYPES_INT32];
    }
    else if (strcmp(argv[4], "INT64") == 0) {
        ptype = &UA_TYPES[UA_TYPES_INT64];
    }
    else if (strcmp(argv[4], "UINT16") == 0) {
        ptype = &UA_TYPES[UA_TYPES_UINT16];
    }
    else if (strcmp(argv[4], "UINT32") == 0) {
        ptype = &UA_TYPES[UA_TYPES_UINT32];
    }
    else if (strcmp(argv[4], "UINT64") == 0) {
        ptype = &UA_TYPES[UA_TYPES_UINT64];
    }
    else if (strcmp(argv[4], "FLOAT") == 0) {
        ptype = &UA_TYPES[UA_TYPES_FLOAT];
    }
    else if (strcmp(argv[4], "DOUBLE") == 0) {
        ptype = &UA_TYPES[UA_TYPES_DOUBLE];
    }
    else if (strcmp(argv[4], "STRING") == 0) {
        ptype = &UA_TYPES[UA_TYPES_STRING];
    }
    else {
        fprintf(stderr, "%s: <Type> can only be BOOL, (U)INT16, (U)INT32, (U)INT64, FLOAT, DOUBLE, STRING.\n", argv[0]);
        fprintf(fw, "XXX-NO-VALUE\n");
        fclose(fw);
        return -5;
    }
    printf("Reading the value of node (%d, %s)...\n", ns, argv[3]);
    UA_Variant *val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(ns, argv[3]), val);
    if (val->data == (void *) NULL) {
            printf("XXX-NO-VALUE\n");
            fprintf(fw, "XXX-NO-VALUE\n");
            fclose(fw);
            result = -6;
            goto end;
    }
    if (val->type != ptype) {
        printf("Actual type: %s.\n", val->type->typeName);
    }
    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
        val->type == ptype) {
        if (strcmp(argv[4], "BOOL") == 0) {
            valueBoolean = *(UA_Boolean *)val->data;
            printf("The value is %hhu.\n", valueBoolean);
            fprintf(fw, "%hhu\n", valueBoolean);
        }
        else if (strcmp(argv[4], "INT16") == 0) {
            valueInt16 = *(UA_Int16 *)val->data;
            printf("The value is %hd.\n", valueInt16);
            fprintf(fw, "%hd\n", valueInt16);
        }
        else if (strcmp(argv[4], "INT32") == 0) {
            valueInt32 = *(UA_Int32 *)val->data;
            printf("The value is %d.\n", valueInt32);
            fprintf(fw, "%d\n", valueInt32);
        }
        else if (strcmp(argv[4], "INT64") == 0) {
            valueInt64 = *(UA_Int64 *)val->data;
            printf("The value is %lld.\n", valueInt64);
            fprintf(fw, "%lld\n", valueInt64);
        }
        else if (strcmp(argv[4], "UINT16") == 0) {
            valueUInt16 = *(UA_UInt16 *)val->data;
            printf("The value is %hu.\n", valueUInt16);
            fprintf(fw, "%hu\n", valueInt16);
        }
        else if (strcmp(argv[4], "UINT32") == 0) {
            valueUInt32 = *(UA_UInt32 *)val->data;
            printf("The value is %u.\n", valueUInt32);
            fprintf(fw, "%u\n", valueInt32);
        }
        else if (strcmp(argv[4], "UINT64") == 0) {
            valueUInt64 = *(UA_UInt64 *)val->data;
            printf("The value is %llu.\n", valueUInt64);
            fprintf(fw, "%llu\n", valueUInt64);
        }
        else if (strcmp(argv[4], "FLOAT") == 0) {
            valueFloat = *(UA_Float *)val->data;
            printf("The value is %f.\n", valueFloat);
            fprintf(fw, "%f\n", valueFloat);
        }
        else if (strcmp(argv[4], "DOUBLE") == 0) {
            valueDouble = *(UA_Double *)val->data;
            printf("The value is %lf.\n", valueDouble);
            fprintf(fw, "%lf\n", valueDouble);
        }
        else if (strcmp(argv[4], "STRING") == 0) {
            ptrValueString = (UA_String *)val->data;
            printf("The value is %s.\n", ptrValueString->data);
            fprintf(fw, "%s\n", ptrValueString->data);
        }
        fclose(fw);
    }
    else {
        printf("XXX-NO-VALUE\n");
        fprintf(fw, "XXX-NO-VALUE\n");
        fclose(fw);
    }
end:
    UA_Variant_delete(val);

    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return result;
}
