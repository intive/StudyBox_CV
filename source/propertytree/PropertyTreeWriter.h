#ifndef PATR_PROPERTY_TREE_WRITER_H
#define PATR_PROPERTY_TREE_WRITER_H

#include <ostream>

class PropertyTree;

/// Dokonuje zapisu drzewa tree do strumienia stream.
/**
 * @param whitespace true jeżeli przy tworzeniu reprezentacji JSON
 * dodawane mają być białe znaki.
 * @throw std::range_error, jeżeli drzewo nie zawiera poprawnych wartości dla JSON.
 */
void WriteJson(const PropertyTree& tree, std::ostream& stream, bool whitespace = true);

namespace Json
{
    class Object;
}

/// Dokonuje konwersji do obiektu Json::Object.
/**
 * Może zostać wykorzystana w specjalizacji PropertyTreeInputConverter<Json::Object>.
 * @throw std::range_error, jeżeli drzewo nie zawiera poprawnych wartości dla JSON.
 */
Json::Object ToJsonObject(const PropertyTree& tree);

#endif // PATR_PROPERTY_TREE_WRITER_H
