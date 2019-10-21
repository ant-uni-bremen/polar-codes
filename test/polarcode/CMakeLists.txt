add_library(PolarTest OBJECT
        siformat
        polarcodetest
        puncturertest
        bitcontainertest
        errordetectiontest
        encodingtest
        decodingtest
        constructiontest)
#install_targets(polarcode-test ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
