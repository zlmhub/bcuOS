#include <ucos_device.h>

#define _OBJ_CONTAINER_LIST_INIT(c)     \
    {&(OS_ObjectContainer[c].ObjectList), &(OS_ObjectContainer[c].ObjectList)}

struct OS_ObjectInformation OS_ObjectContainer[] =
{

    {0, _OBJ_CONTAINER_LIST_INIT(OS_Object_Class_Device), sizeof(struct OS_Device)},

};

/**
 * This function will compare two strings with specified maximum length
 *
 * @param cs the string to be compared
 * @param ct the string to be compared
 * @param count the maximum compare length
 *
 * @return the result
 */
OS_Ubase_T OS_StrNCmp(const char *Cs, const char *Ct, OS_Ubase_T Count)
{
    register signed char __Res = 0;

    while (Count)
    {
        if ((__Res = *Cs - *Ct++) != 0 || !*Cs++)
            break;
        Count --;
    }

    return __Res;
}

/**
 * This function will copy string no more than n bytes.
 *
 * @param dst the string to copy
 * @param src the string to be copied
 * @param n the maximum copied length
 *
 * @return the result
 */
char *OS_StrNCpy(char *Dst, const char *Src, OS_Ubase_T N)
{
    if (N != 0)
    {
        char *D = Dst;
        const char *S = Src;

        do
        {
            if ((*D++ = *S++) == 0)
            {
                /* NUL pad the remaining n-1 bytes */
                while (--N != 0)
                    *D++ = 0;
                break;
            }
        } while (--N != 0);
    }

    return (Dst);
}


/**
 * @brief insert a node after a list
 *
 * @param l list to insert it
 * @param n new node to be inserted
 */
 void OS_ListInsertAfter(OS_List_T *L, OS_List_T *N)
{
    L->Next->Prev = N;
    N->Next = L->Next;

    L->Next = N;
    N->Prev = N;
}


/**
 * This function will initialize an object and add it to object system
 * management.
 *
 * @param object the specified object to be initialized.
 * @param type the object type.
 * @param name the object name. In system, the object's name must be unique.
 */
void OS_ObjectInit(struct OS_Object         *Object,
                    enum OS_ObjectClassType  Type,
                    const char               *Name)
{
    register OS_Base_T Temp;
    struct OS_ObjectInformation *Information;

    /* get object information */
    Information = &OS_ObjectContainer[Type];


    /* initialize object's parameters */

    /* set object type to static */
    Object->Type = Type | OS_Object_Class_Static;

    /* copy name */
    OS_StrNCpy(Object->Name, Name, OS_NAME_MAX);

    //RT_OBJECT_HOOK_CALL(rt_object_attach_hook, (object));


    /* insert object into information object list */
    OS_ListInsertAfter(&(Information->ObjectList), &(Object->List));


}