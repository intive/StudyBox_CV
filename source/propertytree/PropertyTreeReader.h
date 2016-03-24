#ifndef PATR_PROPERTY_TREE_READER_H
#define PATR_PROPERTY_TREE_READER_H

#include <istream>
#include <stdexcept>

class PropertyTree;

/// Dokonuje odczytu drzewa do argumentu tree ze strumienia stream.
/**
 * W chwili obecnej nie ma wsparcia dla Unicode.
 * @throw std::range_error, je¿eli strumieñ nie zawiera poprawnego JSON.
 */
void ReadJson(PropertyTree& tree, std::istream& stream);

#endif // PATR_PROPERTY_TREE_READER_H
