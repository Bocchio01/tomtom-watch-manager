#pragma once
#include <memory>
#include <vector>
#include "tomtom/watch.hpp"

namespace tomtom
{

    class Manager
    {

    public:
        /**
         * @brief Enumerates all connected TomTom watches.
         * @return A vector of shared pointers to Watch instances representing each connected watch.
         */
        static std::vector<std::shared_ptr<Watch>> enumerate();
    };

}
