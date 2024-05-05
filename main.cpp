#include <iostream>
#include <boost/json.hpp>
#include <boost/json/parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "MediaDecoder.h"

using namespace std;

int SDL_main(int argc, char* argv[]) {
    boost::property_tree::ptree config_root;
    try
    {
        boost::property_tree::read_json("D:/MediaServer/Config.json", config_root);
    }
    catch(std::exception& e)
    {
        cout << e.what() << endl;
        return -1;
    }
    int port = config_root.get<int>("port");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

    auto* yuv_render = new YuvRender(854, 480);

    auto* media_decoder = new MediaDecoder(yuv_render);
    media_decoder->open_file("D:/CTTA_1.mp4");
    media_decoder->play();

    delete media_decoder;
    delete yuv_render;

    return 0;
}