#ifndef PATR_PROPERTY_TREE_WRITER_H
#define PATR_PROPERTY_TREE_WRITER_H

#include <ostream>

class PropertyTree;

std::string Unescape(const std::string& input);

/// Dokonuje zapisu drzewa tree do strumienia stream.
/**
 * @param whitespace true jeżeli przy tworzeniu reprezentacji JSON
 * dodawane mają być białe znaki.
 * @throw std::range_error, jeżeli drzewo nie zawiera poprawnych wartości dla JSON.
 */
void WriteJson(const PropertyTree& tree, std::ostream& stream, bool whitespace = true);

class Json;

/// Dokonuje konwersji do obiektu Json::Object.
/**
 * Może zostać wykorzystana w specjalizacji PropertyTreeInputConverter<Json>.
 * @throw std::range_error, jeżeli drzewo nie zawiera poprawnych wartości dla JSON.
 */
Json ToJsonObject(const PropertyTree& tree);

#endif // PATR_PROPERTY_TREE_WRITER_H
