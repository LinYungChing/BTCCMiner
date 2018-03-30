#include "json_utils.hpp"

static std::ostream &output_value(std::ostream &out, json_value *value);

static std::ostream &output_object(std::ostream &out, json_value *value)
{
    if(value == NULL)
        return out;

    out << "{";

    int length = value->u.object.length;

    for(int i=0;i<length;++i)
    {
        if(i != 0)
            out << ", ";
        out << "\"" << value->u.object.values[i].name << "\": ";
        output_value(out, value->u.object.values[i].value);
    }

    out << "}";

    return out;
}

static std::ostream &output_array(std::ostream &out, json_value *value)
{
    if(value == NULL)
        return out;

    out << "[";

    int length = value->u.array.length;
    
    for(int i=0;i<length;++i)
    {
        if(i != 0)
            out << ", ";
        output_value(out, value->u.array.values[i]);
    }

    out << "]";

    return out;
}

static std::ostream &output_value(std::ostream &out, json_value *value)
{
    if(value == NULL)
    {
        return out;
    }

    switch(value->type)
    {
        case json_none:
            out << "none";
            break;
        case json_object:
            output_object(out, value);
            break;
        case json_array:
            output_array(out, value);
            break;
        case json_integer:
            out << value->u.integer;
            break;
        case json_double:
            out << value->u.dbl;
            break;
        case json_string:
            out << value->u.string.ptr;
            break;
        case json_boolean:
            out << (value->u.boolean ? "true":"false");
            break;
        case json_null:
            out << "null";
            break;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, json_value *value)
{
    return output_value(out, value);
}
