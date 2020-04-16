/*
 * This work has been written on the 17th March 2020 by M. Martignano
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

static UA_StatusCode nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
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

#define WRONG_ARGUMENTS_COUNT -1
#define WRONG_OPERATION -2
#define WRONG_ARGUMENTS_COUNT_FOR_OPERATION -3
#define WRONG_NSINDEX -4
#define XXX_NOVALUE -5
#define XXX_NOWRITE -6
#define WRONG_TYPE -7
#define WRONG_STR50_LENGTH -8
#define WRONG_CONSOLE_READ -9
#define WRONG_CONNECTION -10
#define XXX_NOT_ENOUGH_MEMORY -1

#define MY_SELF "si-opc-ua-client"

/*
 * Print usage information
 */
void usage(const char *error) {
    if (strcmp(error, "") != 0) {
        fprintf(stderr, "Error: %s!\n", error);
    }
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s <OPC-UA Server URL> <-r|-w> <Name Space Index> <Node Id> <Type> [<Value>]\n", MY_SELF);
    fprintf(stderr, "valid types are: BOOL, (S)BYTE, (U)INT16, (U)INT32, (U)INT64, FLOAT, DOUBLE, STRING;\n");
    fprintf(stderr, "example:\n");
    fprintf(stderr, "  %s opc.tcp://localhost:4840 -r  1 \"the.answer\" STRING\n", MY_SELF);
    fprintf(stderr, "or:\n");
    fprintf(stderr, "  %s opc.tcp://localhost:4840 -w  1 \"the.answer\" STRING \"42\"\n", MY_SELF);
}

int main(int argc, char *argv[]) {
    int result = 0;

    if ((argc != 6) && (argc != 7)) {
        usage("WRONG_ARGUMENTS_COUNT");
        return WRONG_ARGUMENTS_COUNT;
    }
    if (strcmp(argv[2], "-r") != 0 && strcmp(argv[2], "-w") != 0) {
        usage("WRONG_OPERATION");
        return WRONG_OPERATION;
    }
    if (argc == 6 && strcmp(argv[2], "-r") != 0) {
        usage("WRONG_ARGUMENTS_COUNT_FOR_OPERATION");
        return WRONG_ARGUMENTS_COUNT_FOR_OPERATION;
    }
    if (argc == 7 && strcmp(argv[2], "-w") != 0) {
        usage("WRONG_ARGUMENTS_COUNT_FOR_OPERATION");
        return WRONG_ARGUMENTS_COUNT_FOR_OPERATION;
    }

    int ns = 0;
    UA_Boolean valueBoolean = 0;
    UA_SByte valueSByte = 0;
    UA_Int16 valueInt16 = 0;
    UA_Int32 valueInt32 = 0;
    UA_Int64 valueInt64 = 0;
    UA_Byte valueByte = 0;
    UA_UInt16 valueUInt16 = 0;
    UA_UInt32 valueUInt32 = 0;
    UA_UInt64 valueUInt64 = 0;
    UA_Float valueFloat = 0.0;
    UA_Double valueDouble = 0.0;
    UA_String *ptrValueString;
    if ((sscanf(argv[3], "%d", &ns) != 1) ||
        (ns < 0)) {
        usage("WRONG_NSINDEX");
        return WRONG_NSINDEX;
    }

    const UA_DataType *ptype = (UA_DataType *) NULL;
    /* Read attribute */
    if (strcmp(argv[5], "BOOL") == 0) {
        ptype = &UA_TYPES[UA_TYPES_BOOLEAN];
    } else if (strcmp(argv[5], "SBYTE") == 0) {
        ptype = &UA_TYPES[UA_TYPES_SBYTE];
    } else if (strcmp(argv[5], "INT16") == 0) {
        ptype = &UA_TYPES[UA_TYPES_INT16];
    } else if (strcmp(argv[5], "INT32") == 0) {
        ptype = &UA_TYPES[UA_TYPES_INT32];
    } else if (strcmp(argv[5], "INT64") == 0) {
        ptype = &UA_TYPES[UA_TYPES_INT64];
    } else if (strcmp(argv[5], "BYTE") == 0) {
        ptype = &UA_TYPES[UA_TYPES_BYTE];
    } else if (strcmp(argv[5], "UINT16") == 0) {
        ptype = &UA_TYPES[UA_TYPES_UINT16];
    } else if (strcmp(argv[5], "UINT32") == 0) {
        ptype = &UA_TYPES[UA_TYPES_UINT32];
    } else if (strcmp(argv[5], "UINT64") == 0) {
        ptype = &UA_TYPES[UA_TYPES_UINT64];
    } else if (strcmp(argv[5], "FLOAT") == 0) {
        ptype = &UA_TYPES[UA_TYPES_FLOAT];
    } else if (strcmp(argv[5], "DOUBLE") == 0) {
        ptype = &UA_TYPES[UA_TYPES_DOUBLE];
    } else if (strcmp(argv[5], "STRING") == 0) {
        ptype = &UA_TYPES[UA_TYPES_STRING];
    } else {
        usage("WRONG_TYPE");
        return WRONG_TYPE;
    }

    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    /* Connect to a server */
    /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
    // UA_StatusCode retval = UA_Client_connect_username(client, "opc.tcp://localhost:4840", "user", "password");
    UA_StatusCode retval = UA_Client_connect(client, argv[1]);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        fprintf(stderr, "WRONG_CONNECTION\n");
        return WRONG_CONNECTION;
    }

    UA_Variant *val = UA_Variant_new();
   if (strcmp(argv[2], "-r") == 0) {
        retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
        if (val->data == (void *)NULL) {
            printf("XXX_NOVALUE\n");
            fprintf(stderr, "XXX_NOVALUE\n");
            goto end;
        }
        if (val->type != ptype) {
            printf("Actual type: %s.\n", val->type->typeName);
        }
        if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
            val->type == ptype) {
            if (strcmp(argv[5], "BOOL") == 0) {
                valueBoolean = *(UA_Boolean *)val->data;
                printf("%hhu\n", valueBoolean);
                fprintf(stderr, "READ: %hhu\n", valueBoolean);
            } else if (strcmp(argv[5], "SBYTE") == 0) {
                valueSByte = *(UA_SByte *)val->data;
                printf("%hhd\n", valueSByte);
                fprintf(stderr, "READ: %hhd\n", valueSByte);
            } else if (strcmp(argv[5], "INT16") == 0) {
                valueInt16 = *(UA_Int16 *)val->data;
                printf("%hd\n", valueInt16);
                fprintf(stderr, "READ: %hd\n", valueInt16);
            } else if (strcmp(argv[5], "INT32") == 0) {
                valueInt32 = *(UA_Int32 *)val->data;
                printf("%d\n", valueInt32);
                fprintf(stderr, "READ: %d\n", valueInt32);
            } else if (strcmp(argv[5], "INT64") == 0) {
                valueInt64 = *(UA_Int64 *)val->data;
                printf("%lld\n", valueInt64);
                fprintf(stderr, "READ: %lld\n", valueInt64);
            } else if (strcmp(argv[5], "BYTE") == 0) {
                valueByte = *(UA_Byte *)val->data;
                printf("%hhu\n", valueByte);
                fprintf(stderr, "READ: %hhu\n", valueByte);
            } else if (strcmp(argv[5], "UINT16") == 0) {
                valueUInt16 = *(UA_UInt16 *)val->data;
                printf("%hu\n", valueUInt16);
                fprintf(stderr, "READ: %hu\n", valueInt16);
            } else if (strcmp(argv[5], "UINT32") == 0) {
                valueUInt32 = *(UA_UInt32 *)val->data;
                printf("%u\n", valueUInt32);
                fprintf(stderr, "READ: %u\n", valueInt32);
            } else if (strcmp(argv[5], "UINT64") == 0) {
                valueUInt64 = *(UA_UInt64 *)val->data;
                printf("%llu\n", valueUInt64);
                fprintf(stderr, "READ: %llu\n", valueUInt64);
            } else if (strcmp(argv[5], "FLOAT") == 0) {
                valueFloat = *(UA_Float *)val->data;
                printf("%f\n", valueFloat);
                fprintf(stderr, "READ: %f\n", valueFloat);
            } else if (strcmp(argv[5], "DOUBLE") == 0) {
                valueDouble = *(UA_Double *)val->data;
                printf("%lf\n", valueDouble);
                fprintf(stderr, "READ: %lf\n", valueDouble);
            } else if (strcmp(argv[5], "STRING") == 0) {
                ptrValueString = (UA_String *)val->data;
                UA_Byte * tBuf = malloc(ptrValueString->length + 1);
                if (tBuf == (UA_Byte *) NULL) {
                    printf("XXX_NOT_ENOUGH_MEMORY\n");
                    fprintf(stderr, "XXX_NOT_ENOUGH_MEMORY\n");
                    result = XXX_NOT_ENOUGH_MEMORY;

                } else {
                    for (int i = 0; i <= ptrValueString->length; i++) {
                        tBuf[i] = ptrValueString->data[i];
                    }
                    tBuf[ptrValueString->length] = 0;
                    printf("%s\n", tBuf);
                    fprintf(stderr, "READ: %s\n", tBuf);
                    free(tBuf);
                }
            }
        } else {
            printf("XXX_NOVALUE\n");
            fprintf(stderr, "XXX_NOVALUE\n");
        }
    } else {
        if (strcmp(argv[5], "BOOL") == 0) {
                UA_Boolean value;
                if (sscanf_s(argv[6], "%hhu", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %hhu\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "SBYTE") == 0) {
                UA_SByte value;
                if (sscanf_s(argv[6], "%hhd", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_SBYTE]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %hhd\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "INT16") == 0) {
                UA_Int16 value;
                if (sscanf_s(argv[6], "%hd", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_INT16]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %hd\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "INT32") == 0) {
                UA_Int32 value;
                if (sscanf_s(argv[6], "%d", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_INT32]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %d\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "INT64") == 0) {
                UA_Int64 value;
                if (sscanf_s(argv[6], "%lld", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_INT64]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %lld\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "BYTE") == 0) {
                UA_Byte value;
                if (sscanf_s(argv[6], "%hhu", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_BYTE]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %hhu\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "UINT16") == 0) {
                UA_UInt16 value;
                if (sscanf_s(argv[6], "%hu", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_UINT16]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %hu\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "UINT32") == 0) {
                UA_UInt32 value;
                if (sscanf_s(argv[6], "%u", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_UINT32]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %u\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "UINT64") == 0) {
                UA_UInt64 value;
                if (sscanf_s(argv[6], "%llu", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_UINT64]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %llu\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "FLOAT") == 0) {
                UA_Float value;
                if (sscanf_s(argv[6], "%f", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_FLOAT]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %f\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "DOUBLE") == 0) {
                UA_Double value;
                if (sscanf_s(argv[6], "%lf", &value) == 1) {
                    UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
                    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                    if (retval == UA_STATUSCODE_GOOD) {
                        fprintf(stderr, "WROTE: %lf\n", value);
                    } else {
                        fprintf(stderr, "XXX_NOWRITE\n"); 
                        result = XXX_NOWRITE;
                    }
                } else {
                    usage("WRONG_CONSOLE_READ");
                    result = WRONG_CONSOLE_READ;
                }
            } else if (strcmp(argv[5], "STRING") == 0) {
                UA_String value = UA_STRING_ALLOC(argv[6]);
                UA_Variant_setScalarCopy(val, &value, &UA_TYPES[UA_TYPES_STRING]);
                retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(ns, argv[4]), val);
                if (retval == UA_STATUSCODE_GOOD) {
                    fprintf(stderr, "WROTE: %s\n", argv[6]);
                } else {
                    fprintf(stderr, "XXX_NOWRITE\n"); 
                    result = XXX_NOWRITE;
                }
            }
    }
end:
    UA_Variant_delete(val);

    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return result;
}
