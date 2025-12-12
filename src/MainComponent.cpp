

#include "MainComponent.h"
#include "Identifiers.h"


//===============================================================================================


MainComponent::MainComponent (ValueTree& t) :
                                                tree (t),
                                                gridDisplay (tree, 8, 8,
                                                             { "C", "B", "A", "G", "F", "E", "D", "C" },
                                                             { 12, 11, 9, 7, 5, 4, 2, 0 }),
                                                trainerEngine (tree, 8),
                                                answerChecker (gridDisplay)
{
    setSize (800, 600);

    initializeAudioSettings();
    
    visitComponents ({
        &playButton,
        &gridDisplay,
        &generateButton,
        &submitButton,
        &infoButton,
    }, [this] (Component& c) { addAndMakeVisible (c); });
    
    
    playButton.onClick = [this]()
    {
        trainerEngine.startPlayingMelody();
        playButton.setButtonText ("Play Again");
    };
    
    
    generateButton.onClick = [this]()
    {
        gridDisplay.turnAllTilesOff();
        
        gridDisplay.setSetabilityColumn (0, false);
        
        trainerEngine.generateNextMelody();
        
        playButton.setButtonText ("Start Playing");
        
        auto engine = tree.getChildWithName (IDs::Engine::EngineRoot);
        auto melody = VariantConverter<Melody::Ptr>::fromVar (engine[IDs::Engine::EngineMelody]);
        
        gridDisplay.setStateForTileInColumnWithThisRelativeNote (0,
                                                                 melody->getRelativeFirstNote(),
                                                                 GridDisplayComponent::TileState::tileActive);
    };
    
    
    submitButton.onClick = [this]()
    {
        auto engine = tree.getChildWithName (IDs::Engine::EngineRoot);
        auto engineMelody = VariantConverter<Melody::Ptr>::fromVar (engine[IDs::Engine::EngineMelody]);
        
        answerChecker.compareMelodyToGridState (engineMelody);
    };
    
    infoButton.onClick = [this]()
    {
        auto* infoPanel = new InfoPanelComponent();
        infoPanel->setSize (400, 200);
        CallOutBox::launchAsynchronously (infoPanel, infoButton.getScreenBounds(), nullptr);
    };
    
 
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//===============================================================================================


void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    trainerEngine.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    trainerEngine.getNextAudioBlock (bufferToFill);
}

void MainComponent::releaseResources()
{
    trainerEngine.releaseResources();
}

void MainComponent::initializeAudioSettings()
{
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (0, 2); });
    }
    else
    {
        setAudioChannels (0, 2);
    }
}

//===============================================================================================

void MainComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}


void MainComponent::resized()
{
    setSize (800, 600); // makes window nonresizable
    
    auto r = Rectangle { 50, 25, 200, 50 };
    
    // set bounds for buttons with even spacing
    visitComponents({ &playButton, &generateButton, &submitButton },
                    [&r] (auto& c) { c.setBounds (r); r.translate (250, 0); });
    
    gridDisplay.setBounds ({ 52, 100, 696, 320 });
    
   // answerLabel.setBounds (100, 500, 600, 100);
    infoButton.setBounds (10, 10, 25, 25);
    
    //colourPickButton.setBounds (50, 450, 200, 50);
    
}


