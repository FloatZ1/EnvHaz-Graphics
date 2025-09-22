#ifndef ENVHAZGRAPHICS_SDL_HELPERS_HPP
#define ENVHAZGRAPHICS_SDL_HELPERS_HPP



#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <memory>
#include <stdexcept>
struct SDLDeleter
{

    void operator()(void *p)
    {
        SDL_free(p);
    }
};



class SDLFileReadBuffer
{

  public:
    SDLFileReadBuffer() = default;

    SDLFileReadBuffer(std::string &filepath)
    {
        void *p_data = SDL_LoadFile(filepath.c_str(), &fileSize);
        if (p_data == nullptr)
            throw std::runtime_error("SDLFileReadBuffer had a nullptr reference.\n");

        data.reset(p_data);
        source = std::string((char *)data.get(), fileSize);
    }


    std::string getString() const
    {
        return source;
    }

    void *getData() const
    {
        return data.get();
    }
    size_t getSize() const
    {
        return fileSize;
    }



  private:
    std::unique_ptr<void, SDLDeleter> data;
    size_t fileSize = 0;
    std::string source;
};






#endif
