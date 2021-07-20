#include "program.h"

Program::Program()
{

}

Program::~Program()
{
    ModuleManager::Destroy();
}

void Program::Init()
{
    sf::ContextSettings settings;
    //settings.antialiasingLevel = 8;

    _RenderWindow = new sf::RenderWindow(sf::VideoMode(1024, 768), "Leraine Studio", sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close, settings);
    _RenderWindow->setFramerateLimit(0);
    _RenderWindow->setVerticalSyncEnabled(false);
    _RenderWindow->setActive(true);

    ModuleManager::Init();
    
    RegisterModules();
}

void Program::UpdateWindowMetrics()
{
    _WindowMetrics.Width = _RenderWindow->getView().getSize().x;
    _WindowMetrics.Height = _RenderWindow->getView().getSize().y;

    _WindowMetrics.MiddlePoint = _WindowMetrics.Width / 2;
}

bool Program::HandleEvents()
{
    sf::Event event;
    while (_RenderWindow->pollEvent(event))
    {
        ModuleManager::ProcessEvent(event);
        
        if (event.type == sf::Event::Resized)
        {
            sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
            _RenderWindow->setView(sf::View(visibleArea));
        }

        if (event.type == sf::Event::Closed)
        {
            _RenderWindow->close();
            _ShouldExitProgram = true;
        }
    }

    if(_ShouldExitProgram)
        return false;

    return true;
}

void Program::StartUp()
{
	ModuleManager::StartUp();

    InnerStartUp();
}

void Program::Tick()
{
    UpdateWindowMetrics();

	ModuleManager::Tick(_DeltaClock.getElapsedTime().asSeconds());

    InnerTick();

    _DeltaClock.restart();
}

void Program::Render()
{
	_RenderWindow->clear({ 0,0,0,255 });

	ModuleManager::RenderBack(_RenderWindow);
    InnerRender(_RenderWindow);
	ModuleManager::RenderFront(_RenderWindow);

    _RenderWindow->display();
}

void Program::ShutDown()
{
	ModuleManager::ShutDown();

    InnerShutDown();
}