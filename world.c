#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "world.h"
#include "world_lexer.h"
#include "texture.h"
#include "utils.h"
#include "shaders.h"
#include "shaders_errors.h"
#include "mesh.h"

typedef
enum BlockType
{
    BLOCK_UNKNOWN,
    BLOCK_EOF,
    BLOCK_POINT_LIGHT,
    BLOCK_MATERIAL,
    BLOCK_SQUARE,
    BLOCK_HORIZ_MESH,
    BLOCK_CUBE,
    BLOCK_OPEN_CUBE
}
BlockType;

static const char * blockTypeStr[] =
{
    "", /* BLOCK_UNKNOWN */
    "", /* BLOCK_EOF */
    "PointLight",
    "Material",
    "Square",
    "HorizMesh",
    "Cube",
    "OpenCube"
};

#define BLOCK_TYPES_CNT (sizeof(blockTypeStr) / sizeof(const char *))

typedef
enum AttributeType
{
    ATTR_UNKNOWN,
    ATTR_GLFLOAT,
    ATTR_VEC2,
    ATTR_VEC3,
    ATTR_VEC4,
    ATTR_GLINT,
    ATTR_STRING
}
AttributeType;

static const char * attrTypeStr[] =
{
    "",
    "GLfloat",
    "vec2",
    "vec3",
    "vec4",
    "GLint",
    "string"
};

#define ATTR_TYPES_CNT (sizeof(attrTypeStr) / sizeof(const char *))

typedef
union AttributeValue
{
    GLint v_int;
    GLfloat v_float;
    GLfloat * v_vector;
    char * v_string;
}
AttributeValue;

typedef
struct Attribute
{
    struct Attribute * next;

    AttributeType type;
    char * name;
    AttributeValue value;
    GLsizei cnt;
}
Attribute;

typedef
struct AttributeList
{
    Attribute * first;
    Attribute * last;
}
AttributeList;

/* ---- Functions ---- */

static void die(const char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, fmt, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}

char * getLexNotEof(WorldLexer * lexer)
{
    char * lex = getLex(lexer);

    if (lex == NULL)
    {
        die("getLexNotEof() failed: unexpected EOF.\n");
    }

    return lex;
}

int getInt(WorldLexer * lexer)
{
    char * lex = getLexNotEof(lexer);
    char * endp;
    long int value;

    errno = 0;

    value = strtol(lex, &endp, 10);

    if (errno != 0 || *endp != '\0' ||
        value < INT_MIN || value > INT_MAX)
    {
        free(lex);
        die("Error in getInt().\n");
    }

    free(lex);
    return (int) value;
}

GLfloat getFloat(WorldLexer * lexer)
{
    char * lex = getLexNotEof(lexer);
    char * endp;
    GLfloat f;

    errno = 0;

    f = strtof(lex, &endp);

    if (errno != 0 || *endp != '\0')
    {
        free(lex);
        die("Error in getFloat(), lex: %s.\n", lex);
    }

    free(lex);
    return f;
}

GLfloat * getVec2(WorldLexer * lexer, GLsizei cnt)
{
    GLfloat * vector = (GLfloat *) malloc(cnt * 2 * sizeof(GLfloat));
    int i;

    checkNextLex(lexer, "{");

    for (i = 0; i < cnt; ++i)
    {
        vector[2 * i + 0] = getFloat(lexer);
        vector[2 * i + 1] = getFloat(lexer);
    }

    checkNextLex(lexer, "}");

    return vector;
}


GLfloat * getVec3(WorldLexer * lexer, GLsizei cnt)
{
    GLfloat * vector = (GLfloat *) malloc(cnt * 3 * sizeof(GLfloat));
    int i;

    checkNextLex(lexer, "{");

    for (i = 0; i < cnt; ++i)
    {
        vector[3 * i + 0] = getFloat(lexer);
        vector[3 * i + 1] = getFloat(lexer);
        vector[3 * i + 2] = getFloat(lexer);
    }

    checkNextLex(lexer, "}");

    return vector;
}

GLfloat * getVec4(WorldLexer * lexer, GLsizei cnt)
{
    GLfloat * vector = (GLfloat *) malloc(cnt * 4 * sizeof(GLfloat));
    int i;

    checkNextLex(lexer, "{");

    for (i = 0; i < cnt; ++i)
    {
        vector[4 * i + 0] = getFloat(lexer);
        vector[4 * i + 1] = getFloat(lexer);
        vector[4 * i + 2] = getFloat(lexer);
        vector[4 * i + 3] = getFloat(lexer);
    }

    checkNextLex(lexer, "}");

    return vector;
}

char * getString(WorldLexer * lexer)
{
    char * str;

    checkNextLex(lexer, "\"");

    str = getLexNotEof(lexer);

    checkNextLex(lexer, "\"");

    return str;
}

AttributeType getAttributeType(char * lex)
{
    unsigned int i;

    for (i = 1; i < ATTR_TYPES_CNT; ++i)
    {
        if (STR_EQUAL(lex, attrTypeStr[i]))
        {
            free(lex);
            return (AttributeType) i;
        }
    }

    free(lex);
    die("getAttributeType() failed: unknown attribute type.\n");

    /* Not possible */
    return ATTR_UNKNOWN;
}

Attribute * getAttribute(WorldLexer * lexer)
{
    Attribute * attr;
    char * lex = getLexNotEof(lexer);

    if (STR_EQUAL(lex, "}"))
    {
        free(lex);
        return NULL;
    }

    attr = (Attribute *) malloc(sizeof(Attribute));

    attr->next = NULL;
    attr->type = getAttributeType(lex);
    attr->name = getLexNotEof(lexer);

    lex = getLexNotEof(lexer);

    if (STR_EQUAL(lex, "="))
    {
        attr->cnt = 1;
        free(lex);
    }
    else if (STR_EQUAL(lex, "["))
    {
        free(lex);
        attr->cnt = getInt(lexer);
        checkNextLex(lexer, "]");
        checkNextLex(lexer, "=");
    }
    else
    {
        die("getAttribute() failed.\n");
    }

    switch (attr->type)
    {
        case ATTR_UNKNOWN:
            /* Not possible */
            break;
        case ATTR_GLFLOAT:
            attr->value.v_float = getFloat(lexer);
            break;
        case ATTR_VEC2:
            attr->value.v_vector = getVec2(lexer, attr->cnt);
            break;
        case ATTR_VEC3:
            attr->value.v_vector = getVec3(lexer, attr->cnt);
            break;
        case ATTR_VEC4:
            attr->value.v_vector = getVec4(lexer, attr->cnt);
            break;
        case ATTR_GLINT:
            attr->value.v_int = getInt(lexer);
            break;
        case ATTR_STRING:
            attr->value.v_string = getString(lexer);
            break;
    }

    return attr;
}

int hasAttribute(AttributeList * list, const char * name)
{
    Attribute * cur = list->first;

    while (cur != NULL)
    {
        if (STR_EQUAL(cur->name, name))
        {
            return 1;
        }

        cur = cur->next;
    }

    return 0;
}

AttributeValue getAttributeValue(const AttributeList * list,
    AttributeType type, const char * name, GLsizei cnt)
{
    Attribute * cur = list->first;

    while (cur != NULL)
    {
        if (cur->type == type && STR_EQUAL(cur->name, name) &&
            cur->cnt == cnt)
        {
            return cur->value;
        }

        cur = cur->next;
    }

    die("getAttributeValue() failed: attribute \"%s\" not found.\n",
        name);

    /* Not possible */
    return cur->value;
}

Material * getMaterialByName(const MaterialList * list,
    const char * name)
{
    Material * cur = list->first;

    while (cur != NULL)
    {
        if (STR_EQUAL(cur->name, name))
        {
            return cur;
        }

        cur = cur->next;
    }

    die("getMaterialByName() failed: material \"%s\" not found.\n", name);

    /* Not possible */
    return NULL;
}

AttributeList * getAttributeList(WorldLexer * lexer)
{
    AttributeList * list = (AttributeList *) malloc(sizeof(AttributeList));

    list->first = NULL;
    list->last = NULL;

    checkNextLex(lexer, "{");

    do
    {
        Attribute * attr = getAttribute(lexer);

        if (attr == NULL)
        {
            return list;
        }

        if (hasAttribute(list, attr->name))
        {
            die("getAttributeList() failed: repeated attribute name.\n");
        }

        if (list->last == NULL)
        {
            list->first = list->last = attr;
        }
        else
        {
            list->last = list->last->next = attr;
        }
    }
    while(1);

    return list;
}

void freeAttributeList(AttributeList * list)
{
    Attribute * cur = list->first;
    Attribute * next = NULL;

    while (cur != NULL)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }

    free(list);
}

void copyVec3(GLfloat * dst, const GLfloat * src)
{
    if (src == NULL)
    {
        die("copyVec3() failed.\n");
    }

    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

void copyVec4(GLfloat * dst, const GLfloat * src)
{
    if (src == NULL)
    {
        die("copyVec4() failed.\n");
    }

    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

PointLight * getPointLight(WorldLexer * lexer)
{
    PointLight * pointLight = (PointLight *) malloc(sizeof(PointLight));
    AttributeList * list = getAttributeList(lexer);
    AttributeValue v;

    v = getAttributeValue(list, ATTR_VEC3, "position", 1);
    copyVec3(pointLight->position, v.v_vector);

    v = getAttributeValue(list, ATTR_VEC4, "ambient", 1);
    copyVec4(pointLight->ambient, v.v_vector);

    v = getAttributeValue(list, ATTR_VEC4, "diffuse", 1);
    copyVec4(pointLight->diffuse, v.v_vector);

    v = getAttributeValue(list, ATTR_VEC4, "specular", 1);
    copyVec4(pointLight->specular, v.v_vector);

    v = getAttributeValue(list, ATTR_VEC3, "attenuation", 1);
    copyVec3(pointLight->attenuation, v.v_vector);

    freeAttributeList(list);

    return pointLight;
}

Material * getMaterial(WorldLexer * lexer)
{
    Material * material = (Material *) malloc(sizeof(Material));
    AttributeList * list;
    AttributeValue v;

    material->next = NULL;
    material->name = getLexNotEof(lexer);

    list = getAttributeList(lexer);

    v = getAttributeValue(list, ATTR_STRING, "texture", 1);
    material->textureName = v.v_string;

    v = getAttributeValue(list, ATTR_VEC4, "emission", 1);
    copyVec4(material->emission, v.v_vector);

    v = getAttributeValue(list, ATTR_VEC4, "ambient", 1);
    copyVec4(material->ambient, v.v_vector);

    v = getAttributeValue(list, ATTR_VEC4, "diffuse", 1);
    copyVec4(material->diffuse, v.v_vector);

    v = getAttributeValue(list, ATTR_VEC4, "specular", 1);
    copyVec4(material->specular, v.v_vector);

    v = getAttributeValue(list, ATTR_GLFLOAT, "shininess", 1);
    material->shininess = v.v_float;

    freeAttributeList(list);

    return material;
}

GLfloat * repeatArrayElem(GLfloat * arr, int size, GLsizei cnt, int times)
{
    GLfloat * res = (GLfloat *) malloc(cnt * times * size * sizeof(GLfloat));
    int i, j, k;

    for (i = 0; i < cnt; ++i)
    {
        for (j = 0; j < times; ++j)
        {
            for (k = 0; k < size; ++k)
            {
                int idx = i * times * size + j * size + k;
                res[idx] = arr[i * size + k];
            }
        }
    }

    return res;
}

GLfloat * concatArrays(const GLfloat * arr1, const GLfloat * arr2,
    GLsizei cnt1, GLsizei cnt2)
{
    GLsizei size1 = cnt1 * sizeof(GLfloat);
    GLsizei size2 = cnt2 * sizeof(GLfloat);

    GLfloat * res = (GLfloat *) malloc(size1 + size2);

    memcpy(res, arr1, size1); 
    memcpy(res + cnt1, arr2, size2); 

    return res;
}

WorldObject * getSquare(WorldLexer * lexer)
{
    WorldObject * obj = (WorldObject *) malloc (sizeof(WorldObject));
    AttributeList * list = getAttributeList(lexer);
    AttributeValue v;
    int i;

    obj->next = NULL;

    v = getAttributeValue(list, ATTR_STRING, "material", 1);
    obj->materialName = v.v_string;

    obj->primitiveType = GL_TRIANGLES;
    obj->cnt = 4;

    v = getAttributeValue(list, ATTR_VEC3, "corners", obj->cnt);
    obj->position = v.v_vector;

    v = getAttributeValue(list, ATTR_VEC3, "normal", 1);
    obj->normal = repeatArrayElem(v.v_vector, 3, 1, obj->cnt);

    v = getAttributeValue(list, ATTR_VEC2, "texture_coords", obj->cnt);
    obj->texCoord = v.v_vector;

    obj->idxCnt = 6;
    obj->idx = (GLuint *) malloc(obj->idxCnt * sizeof(GLuint));

    for (i = 0; i < 3; ++i)
    {
        obj->idx[i] = i;
    }

    for (i = 3; i < 6; ++i)
    {
        obj->idx[i] = i - 2;
    }

    return obj;
}

WorldObject * getHorizMesh(WorldLexer * lexer)
{
    WorldObject * obj = (WorldObject *) malloc (sizeof(WorldObject));
    AttributeList * list = getAttributeList(lexer);
    AttributeValue v;
    MeshData data;
    int x, y;

    obj->next = NULL;

    v = getAttributeValue(list, ATTR_STRING, "material", 1);
    obj->materialName = v.v_string;

    obj->primitiveType = GL_TRIANGLES;

    v = getAttributeValue(list, ATTR_GLINT, "w", 1);
    data.w = v.v_int;

    v = getAttributeValue(list, ATTR_GLINT, "h", 1);
    data.h = v.v_int;

    obj->cnt = data.w * data.h;

    v = getAttributeValue(list, ATTR_GLFLOAT, "firstX", 1);
    data.firstX = v.v_float;

    v = getAttributeValue(list, ATTR_GLFLOAT, "lastX", 1);
    data.lastX = v.v_float;

    v = getAttributeValue(list, ATTR_GLFLOAT, "firstY", 1);
    data.firstY = v.v_float;

    v = getAttributeValue(list, ATTR_GLFLOAT, "lastY", 1);
    data.lastY = v.v_float;

    v = getAttributeValue(list, ATTR_GLFLOAT, "z", 1);
    data.z = v.v_float;

    obj->position = meshGenVertices(&data);
    obj->idxCnt = meshGenIdx(&data, &(obj->idx));

    v = getAttributeValue(list, ATTR_VEC3, "normal", 1);
    obj->normal = repeatArrayElem(v.v_vector, 3, 1, obj->cnt);

    v = getAttributeValue(list, ATTR_VEC2, "texture_coords_from_to", 2);
    obj->texCoord = (GLfloat *) malloc(obj->cnt * 2 * sizeof(GLfloat));

    for (y = 0; y < data.w; ++y)
    {
        for (x = 0; x < data.h; ++x)
        {
            obj->texCoord[2 * (y * data.w + x) + 0] =
                (v.v_vector)[0] + ((v.v_vector)[2] - v.v_vector[0]) *
                x / (data.w - 1);
            obj->texCoord[2 * (y * data.w + x) + 1] =
                (v.v_vector)[1] + ((v.v_vector)[3] - v.v_vector[1]) *
                y / (data.h - 1);
        }
    }

    return obj;
}

GLfloat * getOpenCubePosition(GLfloat * top, GLfloat * bottom)
{
    GLfloat * position = (GLfloat *) malloc(20 * 3 * sizeof(GLfloat));

    /* bottom */
    copyVec3(position + 3 * 0, bottom + 3 * 0);
    copyVec3(position + 3 * 1, bottom + 3 * 1);
    copyVec3(position + 3 * 2, bottom + 3 * 2);
    copyVec3(position + 3 * 3, bottom + 3 * 3);

    /* left */
    copyVec3(position + 3 * 4, top + 3 * 0);
    copyVec3(position + 3 * 5, top + 3 * 1);
    copyVec3(position + 3 * 6, bottom + 3 * 0);
    copyVec3(position + 3 * 7, bottom + 3 * 1);

    /* right */
    copyVec3(position + 3 *  8, top + 3 * 2);
    copyVec3(position + 3 *  9, top + 3 * 3);
    copyVec3(position + 3 * 10, bottom + 3 * 2);
    copyVec3(position + 3 * 11, bottom + 3 * 3);

    /* back */
    copyVec3(position + 3 * 12, top + 3 * 1);
    copyVec3(position + 3 * 13, top + 3 * 3);
    copyVec3(position + 3 * 14, bottom + 3 * 1);
    copyVec3(position + 3 * 15, bottom + 3 * 3);

    /* forward */
    copyVec3(position + 3 * 16, top + 3 * 0);
    copyVec3(position + 3 * 17, top + 3 * 2);
    copyVec3(position + 3 * 18, bottom + 3 * 0);
    copyVec3(position + 3 * 19, bottom + 3 * 2);

    return position;
}

GLfloat * getCubePosition(GLfloat * top, GLfloat * bottom)
{
    GLfloat * position = (GLfloat *) malloc(24 * 3 * sizeof(GLfloat));

    /* top */
    copyVec3(position + 3 * 0, top + 3 * 0);
    copyVec3(position + 3 * 1, top + 3 * 1);
    copyVec3(position + 3 * 2, top + 3 * 2);
    copyVec3(position + 3 * 3, top + 3 * 3);


    /* bottom */
    copyVec3(position + 3 * 4, bottom + 3 * 0);
    copyVec3(position + 3 * 5, bottom + 3 * 1);
    copyVec3(position + 3 * 6, bottom + 3 * 2);
    copyVec3(position + 3 * 7, bottom + 3 * 3);

    /* left */
    copyVec3(position + 3 *  8, top + 3 * 0);
    copyVec3(position + 3 *  9, top + 3 * 1);
    copyVec3(position + 3 * 10, bottom + 3 * 0);
    copyVec3(position + 3 * 11, bottom + 3 * 1);

    /* right */
    copyVec3(position + 3 * 12, top + 3 * 2);
    copyVec3(position + 3 * 13, top + 3 * 3);
    copyVec3(position + 3 * 14, bottom + 3 * 2);
    copyVec3(position + 3 * 15, bottom + 3 * 3);

    /* back */
    copyVec3(position + 3 * 16, top + 3 * 1);
    copyVec3(position + 3 * 17, top + 3 * 3);
    copyVec3(position + 3 * 18, bottom + 3 * 1);
    copyVec3(position + 3 * 19, bottom + 3 * 3);

    /* forward */
    copyVec3(position + 3 * 20, top + 3 * 0);
    copyVec3(position + 3 * 21, top + 3 * 2);
    copyVec3(position + 3 * 22, bottom + 3 * 0);
    copyVec3(position + 3 * 23, bottom + 3 * 2);

    return position;
}

GLfloat * getOpenCubeTexCoord(const AttributeList * list)
{
    AttributeValue v;
    GLfloat * bottomCoord;
    GLfloat * edgeCoord;
    GLfloat * res;

    v = getAttributeValue(list, ATTR_VEC2, "texture_coords_bottom", 4);
    bottomCoord = v.v_vector;

    v = getAttributeValue(list, ATTR_VEC2, "texture_coords", 4);
    edgeCoord = repeatArrayElem(v.v_vector, 8, 1, 4);

    res = concatArrays(bottomCoord, edgeCoord, 8, 32);

    free(edgeCoord);

    return res;
}

GLfloat * getCubeTexCoord(const AttributeList * list)
{
    AttributeValue v;

    v = getAttributeValue(list, ATTR_VEC2, "texture_coords", 4);
    return repeatArrayElem(v.v_vector, 8, 1, 6);
}


WorldObject * getOpenCube(WorldLexer * lexer)
{
    WorldObject * obj = (WorldObject *) malloc (sizeof(WorldObject));
    AttributeList * list = getAttributeList(lexer);
    GLfloat * top;
    GLfloat * bottom;
    AttributeValue v;
    int i;

    obj->next = NULL;

    v = getAttributeValue(list, ATTR_STRING, "material", 1);
    obj->materialName = v.v_string;

    obj->primitiveType = GL_TRIANGLES;
    obj->cnt = 20;

    v = getAttributeValue(list, ATTR_VEC3, "top", 4);
    top = v.v_vector;

    v = getAttributeValue(list, ATTR_VEC3, "bottom", 4);
    bottom = v.v_vector;

    obj->position = getOpenCubePosition(top, bottom);

    v = getAttributeValue(list, ATTR_VEC3, "normal", 5);
    obj->normal = repeatArrayElem(v.v_vector, 3, 5, 4);

    obj->texCoord = getOpenCubeTexCoord(list);

    obj->idxCnt = 30;
    obj->idx = (GLuint *) malloc(obj->idxCnt * sizeof(GLuint));

    for (i = 0; i < 3; ++i)
    {
        obj->idx[i] = i;
    }

    for (i = 3; i < 6; ++i)
    {
        obj->idx[i] = i - 2;
    }

    for (i = 6; i < obj->idxCnt; ++i)
    {
        obj->idx[i] = obj->idx[i % 6] + 4 * (i / 6);
    }

    return obj;
}

WorldObject * getCube(WorldLexer * lexer)
{
    WorldObject * obj = (WorldObject *) malloc (sizeof(WorldObject));
    AttributeList * list = getAttributeList(lexer);
    GLfloat * top;
    GLfloat * bottom;
    AttributeValue v;
    int i;

    obj->next = NULL;

    v = getAttributeValue(list, ATTR_STRING, "material", 1);
    obj->materialName = v.v_string;

    obj->primitiveType = GL_TRIANGLES;
    obj->cnt = 24;

    v = getAttributeValue(list, ATTR_VEC3, "top", 4);
    top = v.v_vector;

    v = getAttributeValue(list, ATTR_VEC3, "bottom", 4);
    bottom = v.v_vector;

    obj->position = getCubePosition(top, bottom);

    v = getAttributeValue(list, ATTR_VEC3, "normal", 6);
    obj->normal = repeatArrayElem(v.v_vector, 3, 6, 4);

    obj->texCoord = getCubeTexCoord(list);

    obj->idxCnt = 36;
    obj->idx = (GLuint *) malloc(obj->idxCnt * sizeof(GLuint));

    for (i = 0; i < 3; ++i)
    {
        obj->idx[i] = i;
    }

    for (i = 3; i < 6; ++i)
    {
        obj->idx[i] = i - 2;
    }

    for (i = 6; i < obj->idxCnt; ++i)
    {
        obj->idx[i] = obj->idx[i % 6] + 4 * (i / 6);
    }

    return obj;
}

void addPointLight(PointLight ** pointLightP, PointLight * pointLight)
{
    if (*pointLightP == NULL)
    {
        *pointLightP = pointLight;
    }
    else
    {
        die("Supported only one point light.\n");
    }
}

void addMaterial(MaterialList * list, Material * material)
{
    if (list->last == NULL)
    {
        list->first = list->last = material;
    }
    else
    {
        list->last = list->last->next = material;
    }
}

void addWorldObject(WorldObjectList * list, WorldObject * object)
{
    if (list->last == NULL)
    {
        list->first = list->last = object;
    }
    else
    {
        list->last = list->last->next = object;
    }
}

void addTexture(TextureList * list, Texture * texture)
{
    if (list->last == NULL)
    {
        list->first = list->last = texture;
    }
    else
    {
        list->last = list->last->next = texture;
    }

    ++(list->cnt);
}

Texture * getTextureByName(const TextureList * list,
    const char * name)
{
    Texture * cur = list->first;

    while (cur != NULL)
    {
        if (STR_EQUAL(cur->name, name))
        {
            return cur;
        }

        cur = cur->next;
    }

    return NULL;
}

BlockType getBlockType(char * lex)
{
    unsigned int i;

    for (i = 1; i < BLOCK_TYPES_CNT; ++i)
    {
        if (STR_EQUAL(lex, blockTypeStr[i]))
        {
            free(lex);
            return (BlockType) i;
        }
    }

    free(lex);
    return BLOCK_UNKNOWN;
}

void freeWorld(World * world)
{
    freeShaderProgram(world->sp);

    /* TODO: free lists and pointLight. */
}

void setupPointLight(ShaderProgram * sp, const PointLight * pointLight)
{
    GLint obj;

    obj = glGetUniformLocation(sp->p, "pointLight.position");
    /* TODO: if (obj == -1) {} */
    glUniform3fv(obj, 1, pointLight->position);

    obj = glGetUniformLocation(sp->p, "pointLight.ambient");
    /* TODO: if (obj == -1) {} */
    glUniform4fv(obj, 1, pointLight->ambient);

    obj = glGetUniformLocation(sp->p, "pointLight.diffuse");
    /* TODO: if (obj == -1) {} */
    glUniform4fv(obj, 1, pointLight->diffuse);

    obj = glGetUniformLocation(sp->p, "pointLight.specular");
    /* TODO: if (obj == -1) {} */
    glUniform4fv(obj, 1, pointLight->specular);

    obj = glGetUniformLocation(sp->p, "pointLight.attenuation");
    /* TODO: if (obj == -1) {} */
    glUniform3fv(obj, 1, pointLight->attenuation);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

Texture * getTexture(const char * name, int num)
{
    Texture * res;
    
    if (STR_EQUAL(name, "EMPTY"))
    {
        return NULL;
    }

    res = (Texture *) malloc(sizeof(Texture));

    res->next = NULL;
    res->name = name;
    res->id = createTextureFromTga(name);

    if (res->id == 0)
    {
        die("Failed to load texture from file.\n");
    }

    res->num = num;

    return res;
}

void setupTexture(ShaderProgram * sp, TextureList * texList,
    const char * name)
{
    Texture * texture = getTextureByName(texList, name);

    glUseProgram(sp->p);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void setupTexture_Unit1(ShaderProgram * sp, TextureList * texList,
    const char * name)
{
    Texture * texture = getTextureByName(texList, name);
    GLint obj;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    obj = glGetUniformLocation(sp->p, "texPool");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, 1);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void setupTexture_Unit5(ShaderProgram * sp, TextureList * texList,
    const char * name)
{
    Texture * texture = getTextureByName(texList, name);
    GLint obj;

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    obj = glGetUniformLocation(sp->p, "texCube");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, 5);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void setupMaterial(ShaderProgram * sp, const Material * material)
{
    GLint obj;

    obj = glGetUniformLocation(sp->p, "material.emission");
    /* TODO: if (obj == -1) {} */
    glUniform4fv(obj, 1, material->emission);

    obj = glGetUniformLocation(sp->p, "material.ambient");
    /* TODO: if (obj == -1) {} */
    glUniform4fv(obj, 1, material->ambient);

    obj = glGetUniformLocation(sp->p, "material.diffuse");
    /* TODO: if (obj == -1) {} */
    glUniform4fv(obj, 1, material->diffuse);

    obj = glGetUniformLocation(sp->p, "material.specular");
    /* TODO: if (obj == -1) {} */
    glUniform4fv(obj, 1, material->specular);

    obj = glGetUniformLocation(sp->p, "material.shininess");
    /* TODO: if (obj == -1) {} */
    glUniform1f(obj, material->shininess);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

void setupWorldObject(ShaderProgram * sp, WorldObject * obj)
{
    glGenVertexArrays(1, &(obj->vaoP));
    glBindVertexArray(obj->vaoP);

    setupVbo(sp, obj->position, "position", 3, obj->cnt);
    setupVbo(sp, obj->normal, "normal", 3, obj->cnt);
    setupVbo(sp, obj->texCoord, "texCoord", 2, obj->cnt);

    setupIdxVbo(sp, obj->idx, obj->idxCnt);
}

void setupWorldShaderProgram(World * world)
{
    WorldObject * cur = world->objList.first;
    GLuint obj;

    world->sp = getShaderProgram("draw_world_vshader.glsl",
        NULL, "draw_world_fshader.glsl");

    setupPointLight(world->sp, world->pointLight);

    while (cur != NULL)
    {
        setupWorldObject(world->sp, cur);
        cur = cur->next;
    }

    obj = glGetUniformLocation(world->sp->p, "texSampler");
    /* TODO: if (obj == -1) {} */
    glUniform1i(obj, 0);

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}

World * getWorld(const char * path)
{
    WorldLexer * lexer = newWorldLexer(path);
    World * world = (World *) malloc(sizeof(World));

    world->pointLight = NULL;
    world->mtrlList.first = NULL;
    world->mtrlList.last = NULL;
    world->objList.first = NULL;
    world->objList.last = NULL;
    world->texList.first = NULL;
    world->texList.last = NULL;
    world->texList.cnt = 0;

    do
    {
        char * lex = getLex(lexer);
        Material * material;
        Texture * texture;

        if (lex == NULL)
        {
            /* EOF situation */
            break;
        }

        switch (getBlockType(lex))
        {
            case BLOCK_UNKNOWN:
                die("Unknown block type.\n");
            case BLOCK_EOF:
                goto to_ret;
            case BLOCK_POINT_LIGHT:
                addPointLight(&(world->pointLight), getPointLight(lexer));
                break;
            case BLOCK_MATERIAL:
                material = getMaterial(lexer);
                addMaterial(&(world->mtrlList), material);

                texture = getTexture(world->mtrlList.last->textureName,
                    world->texList.cnt);

                if (texture != NULL)
                {
                    addTexture(&(world->texList), texture);
                }
                break;
            case BLOCK_SQUARE:
                addWorldObject(&(world->objList), getSquare(lexer));
                break;
            case BLOCK_HORIZ_MESH:
                addWorldObject(&(world->objList), getHorizMesh(lexer));
                break;
            case BLOCK_CUBE:
                addWorldObject(&(world->objList), getCube(lexer));
                break;
            case BLOCK_OPEN_CUBE:
                addWorldObject(&(world->objList), getOpenCube(lexer));
                break;
        }
    }
    while (1);

to_ret:
    freeWorldLexer(lexer);
    setupWorldShaderProgram(world);
    return world;
}

void setupWater(ShaderProgram * sp, World * world)
{
    Material * material =
        getMaterialByName(&(world->mtrlList), "for_water");

    setupPointLight(sp, world->pointLight);
    setupMaterial(sp, material);

    setupTexture_Unit1(sp, &(world->texList), "texture_pool.tga");
    setupTexture_Unit5(sp, &(world->texList), "texture.tga");
}

void drawWorld(World * world)
{
    Material * material;

    WorldObject * cur = world->objList.first;

    glUseProgram(world->sp->p);

    while (cur != NULL)
    {
        glBindVertexArray(cur->vaoP);

        material = getMaterialByName(&(world->mtrlList), cur->materialName);
        setupTexture(world->sp, &(world->texList), material->textureName);
        setupMaterial(world->sp, material);

        glDrawElements(cur->primitiveType, cur->idxCnt,
            GL_UNSIGNED_INT, NULL);

        cur = cur->next;
    }

    CHECK_OPENGL_ERRORS(__FILE__, __LINE__);
}
