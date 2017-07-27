add_library(PolarTest OBJECT
        polarcodetest
        bitcontainertest
        errordetectiontest
        encodingtest
        decodingtest
        constructiontest)
#install_targets(polarcode-test ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
