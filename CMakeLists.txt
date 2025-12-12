cmake_minimum_required(VERSION 3.22)
project(GREG_TRAINER VERSION 0.0.1)

juce_add_gui_app(GregTrainer
    # VERSION ...                       # Set this if the app version is different to the project version
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    # COMPANY_NAME ...                  # Specify the name of the app's author
    PRODUCT_NAME "GregTrainer"
)

target_sources(GregTrainer
    PRIVATE
        src/AnswerChecker.h
        src/ExtraMenus.h
        src/GridDisplayComponent.cpp
        src/GridDisplayComponent.h
        src/Identifiers.h
        src/Main.cpp
        src/MainComponent.cpp
        src/MainComponent.h
        src/MelodyGenerator.h
        src/MidiGenerator.h
        src/Synth.h
        src/TrainerEngine.cpp
        src/TrainerEngine.h
        src/Utility.h
)

target_compile_definitions(GregTrainer
    PRIVATE
        JUCE_WEB_BROWSER=0 
        JUCE_USE_CURL=0   
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:GregTrainer,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:GregTrainer,JUCE_VERSION>"
)

target_link_libraries(GregTrainer
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
