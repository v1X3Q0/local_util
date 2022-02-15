#ifndef LOCALUTIL_CPP_H
#define LOCALUTIL_CPP_H

#include <vector>
#include <algorithm>

void dumpMem(uint8_t* base, size_t len, char format);

template<typename t, typename u>
int vector_pair_ind(std::vector<std::pair<t, u>>* toSearch, t lookupKey)
{
    int indexOut = 0;

    for (auto i = toSearch->begin(); i != toSearch->end(); i++)
    {
        if (i->first == lookupKey)
        {
            return indexOut;
        }
        indexOut++;
    }
    return -1;
}

template<typename t, typename u>
u vector_pair_key_find(std::vector<std::pair<t, u>>* targetVector, t lookupKey)
{
    int newInd = vector_pair_ind<t, u>(targetVector, lookupKey);
    if (newInd == -1)
    {
        return 0;
    }
    else
    {
        return (*targetVector)[newInd].second;
    }
}

template<typename t, typename u>
void vector_pair_sort(std::vector<std::pair<t, u>>* targetVector,
    bool (*cmp)(std::pair<t, u>&, std::pair<t, u>&))
{
    std::sort(targetVector->begin(), targetVector->end(), cmp);
}

#endif