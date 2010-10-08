/**

Game Develop - Particle System Extension
Copyright (c) 2010 Florian Rival (Florian.Rival@gmail.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.

*/

#include <SFML/Graphics.hpp>
#include "GDL/Object.h"

#include "GDL/ImageManager.h"
#include "GDL/tinyxml.h"
#include "GDL/FontManager.h"
#include "GDL/Position.h"
#include "ParticleEmitterObject.h"

#ifdef GDE
#include <wx/wx.h>
#include "GDL/CommonTools.h"
#include "GDL/ResourcesMergingHelper.h"
#include "GDL/MainEditorCommand.h"
#include "ParticleEmitterObjectEditor.h"
#endif

bool ParticleEmitterObject::SPKinitialized = false;

ParticleEmitterObject::ParticleEmitterObject(std::string name_) :
Object(name_),
baseParticleSystemID(SPK::NO_ID),
particleSystem(NULL),
rendererType(Point),
opacity( 255 ),
colorR( 255 ),
colorG( 255 ),
colorB( 255 ),
angle(0)
{
    if ( !SPKinitialized )
    {
        SPK::randomSeed = static_cast<unsigned int>(time(NULL));
        SPK::System::setClampStep(true,0.1f);			// clamp the step to 100 ms
        SPK::System::useAdaptiveStep(0.001f,0.01f);		// use an adaptive step from 1ms to 10ms (1000fps to 100fps)

        SPK::SFML::SFMLRenderer::setZFactor(1.0f);
        SPK::SFML::setCameraPosition(SPK::SFML::CAMERA_CENTER,SPK::SFML::CAMERA_BOTTOM,static_cast<float>(1440),0.0f);

        SPKinitialized = true;
    }
}

void ParticleEmitterObject::LoadFromXml(const TiXmlElement * object)
{
    if ( object->FirstChildElement( "Color" ) == NULL ||
         object->FirstChildElement( "Color" )->Attribute("r") == NULL ||
         object->FirstChildElement( "Color" )->Attribute("g") == NULL ||
         object->FirstChildElement( "Color" )->Attribute("b") == NULL )
    {
        cout << "Les informations concernant la couleur du texte d'un objet Text manquent.";
    }
    else
    {
        int r = 255;
        int g = 255;
        int b = 255;
        object->FirstChildElement("Color")->QueryIntAttribute("r", &r);
        object->FirstChildElement("Color")->QueryIntAttribute("g", &g);
        object->FirstChildElement("Color")->QueryIntAttribute("b", &b);

        SetColor(r,g,b);
    }
}

#if defined(GDE)
void ParticleEmitterObject::SaveToXml(TiXmlElement * object)
{
    TiXmlElement * color = new TiXmlElement( "Color" );
    object->LinkEndChild( color );
    color->SetAttribute("r", GetColorR());
    color->SetAttribute("g", GetColorG());
    color->SetAttribute("b", GetColorB());
}
#endif

// creates and register the base system
SPK::SPK_ID ParticleEmitterObject::CreateBaseParticleSystem()
{
	// Creates the model
	SPK::Model* sparkModel = SPK::Model::create(SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
		SPK::FLAG_ALPHA,
		SPK::FLAG_GREEN | SPK::FLAG_BLUE);
	sparkModel->setParam(SPK::PARAM_RED,1.0f);
	sparkModel->setParam(SPK::PARAM_BLUE,0.0f,0.2f);
	sparkModel->setParam(SPK::PARAM_GREEN,0.2f,1.0f);
	sparkModel->setParam(SPK::PARAM_ALPHA,0.8f,0.0f);
	sparkModel->setLifeTime(0.6f,3.6f);

	// Creates the renderer
	SPK::SFML::SFMLRenderer * renderer = NULL;
	if ( rendererType == Line )
	{
	    renderer = SPK::SFML::SFMLLineRenderer::create(0.15f,2.0f);
	}
	else
	{
	    renderer = SPK::SFML::SFMLPointRenderer::create(1.5f);
	}
	renderer->setGroundCulling(true);
	renderer->setBlendMode(sf::Blend::Add);

	// Creates the zone
	SPK::Sphere* sparkSource = SPK::Sphere::create(SPK::Vector3D(0.0f,0.0f,10.0f),5.0f);

	// Creates the emitter
	SPK::SphericEmitter* emitter = SPK::SphericEmitter::create(SPK::Vector3D(0.0f,0.0f,1.0f),3.14159f / 4.0f,3.0f * 3.14159f / 4.0f);
	emitter->setForce(70.0f,190.0f);
	emitter->setZone(sparkSource);
	emitter->setTank(-1);
	emitter->setFlow(300);

	// Creates the Group
	SPK::Group* sparkGroup = SPK::Group::create(sparkModel);
	sparkGroup->setRenderer(renderer);
	sparkGroup->addEmitter(emitter);
	sparkGroup->setGravity(SPK::Vector3D(0.0f,0.0f,100.0f));
	sparkGroup->setFriction(2.0f);

	// Creates the System
	SPK::SFML::SFMLSystem* sparkSystem = SPK::SFML::SFMLSystem::create();
	sparkSystem->addGroup(sparkGroup);

	// Defines which objects will be shared by all systems
	sparkModel->setShared(true);
	renderer->setShared(true);

	// Creates the base and gets the ID
	return sparkSystem->getID();
}

bool ParticleEmitterObject::LoadRuntimeResources(const ImageManager & imageMgr )
{
	// Loads particle texture
	if (!textureParticle.LoadFromFile("D:/Florian/Programmation/GameDevelop/Extensions/ParticleSystem/SPARK/demos/bin/res/flare.png"))
		cout << "loading error2";

	baseParticleSystemID = CreateBaseParticleSystem();
    particleSystem = SPK_Copy(SPK::SFML::SFMLSystem, baseParticleSystemID);
	particleSystem->SetPosition(GetX(), GetY());

    return true;
}

bool ParticleEmitterObject::LoadResources(const ImageManager & imageMgr)
{
    #if defined(GDE)
    edittimeIconImage.LoadFromFile("Extensions/particleSystemicon.png");
    edittimeIcon.SetImage(edittimeIconImage);
    #endif

    return true;
}

ParticleEmitterObject::~ParticleEmitterObject()
{
    if ( particleSystem ) SPK_Destroy(particleSystem);
}

/**
 * Update animation and direction from the inital position
 */
bool ParticleEmitterObject::InitializeFromInitialPosition(const InitialPosition & position)
{
    return true;
}

/**
 * Render object at runtime
 */
bool ParticleEmitterObject::Draw( sf::RenderWindow& window )
{
    //Don't draw anything if hidden
    if ( hidden ) return true;

    window.RestoreGLStates();
    window.Draw(*particleSystem);
    window.SaveGLStates();

    return true;
}

#ifdef GDE
/**
 * Render object at edittime
 */
bool ParticleEmitterObject::DrawEdittime(sf::RenderWindow& renderWindow)
{
    edittimeIcon.SetPosition(GetX(), GetY());
    renderWindow.Draw(edittimeIcon);

    return true;
}

void ParticleEmitterObject::PrepareResourcesForMerging(ResourcesMergingHelper & resourcesMergingHelper)
{
}

bool ParticleEmitterObject::GenerateThumbnail(const Game & game, wxBitmap & thumbnail)
{
    thumbnail = wxBitmap("Extensions/particleSystemicon24.png", wxBITMAP_TYPE_ANY);

    return true;
}

void ParticleEmitterObject::EditObject( wxWindow* parent, Game & game, MainEditorCommand & mainEditorCommand )
{
    ParticleEmitterObjectEditor dialog(parent, game, *this, mainEditorCommand);
    dialog.ShowModal();
}

wxPanel * ParticleEmitterObject::CreateInitialPositionPanel( wxWindow* parent, const Game & game_, const Scene & scene_, const InitialPosition & position )
{
    return NULL;
}

void ParticleEmitterObject::UpdateInitialPositionFromPanel(wxPanel * panel, InitialPosition & position)
{
}

void ParticleEmitterObject::GetPropertyForDebugger(unsigned int propertyNb, string & name, string & value) const
{
    if ( !particleSystem ) return;

    if      ( propertyNb == 0 ) {name = _("Nombre de particules");      value = ToString(particleSystem->getNbParticles());}
    else if ( propertyNb == 1 ) {name = _("Couleur");       value = ToString(GetColorR())+";"+ToString(GetColorG())+";"+ToString(GetColorB());}
    else if ( propertyNb == 2 ) {name = _("Opacit�");       value = ToString(GetOpacity());}
}

bool ParticleEmitterObject::ChangeProperty(unsigned int propertyNb, string newValue)
{
    if      ( propertyNb == 0 ) { return false; }
    else if ( propertyNb == 1 )
    {
        string r, gb, g, b;
        {
            size_t separationPos = newValue.find(";");

            if ( separationPos > newValue.length())
                return false;

            r = newValue.substr(0, separationPos);
            gb = newValue.substr(separationPos+1, newValue.length());
        }

        {
            size_t separationPos = gb.find(";");

            if ( separationPos > gb.length())
                return false;

            g = gb.substr(0, separationPos);
            b = gb.substr(separationPos+1, gb.length());
        }

        SetColor(ToInt(r), ToInt(g), ToInt(b));
    }
    else if ( propertyNb == 2 ) { SetOpacity(ToFloat(newValue)); }

    return true;
}

unsigned int ParticleEmitterObject::GetNumberOfProperties() const
{
    return 3;
}
#endif

void ParticleEmitterObject::OnPositionChanged()
{
    if ( particleSystem )
        particleSystem->SetPosition(GetX(), GetY());
}

/**
 * Get the real X position of the sprite
 */
float ParticleEmitterObject::GetDrawableX() const
{
    return GetX();
}

/**
 * Get the real Y position of the text
 */
float ParticleEmitterObject::GetDrawableY() const
{
    return GetY();
}

/**
 * Width is the width of the current sprite.
 */
float ParticleEmitterObject::GetWidth() const
{
    return 32;
}

/**
 * Height is the height of the current sprite.
 */
float ParticleEmitterObject::GetHeight() const
{
    return 32;
}

/**
 * X center is computed with text rectangle
 */
float ParticleEmitterObject::GetCenterX() const
{
    return 16;
}

/**
 * Y center is computed with text rectangle
 */
float ParticleEmitterObject::GetCenterY() const
{
    return 16;
}

/**
 * Nothing to do when updating time
 */
void ParticleEmitterObject::UpdateTime(float deltaTime)
{
	particleSystem->update (deltaTime);
}

/**
 * Change the color filter of the sprite object
 */
void ParticleEmitterObject::SetColor( unsigned int r, unsigned int g, unsigned int b )
{
    colorR = r;
    colorG = g;
    colorB = b;
    if ( particleSystem ) particleSystem->SetColor(sf::Color(colorR, colorG, colorB, opacity));
}

void ParticleEmitterObject::SetOpacity(float val)
{
    if ( val > 255 )
        val = 255;
    else if ( val < 0 )
        val = 0;

    opacity = val;
    if ( particleSystem ) particleSystem->SetColor(sf::Color(colorR, colorG, colorB, opacity));
}

/**
 * Function destroying an extension Object.
 * Game Develop does not delete directly extension object
 * to avoid overloaded new/delete conflicts.
 */
void DestroyParticleEmitterObject(Object * object)
{
    delete object;
}

/**
 * Function creating an extension Object.
 * Game Develop can not directly create an extension object
 */
Object * CreateParticleEmitterObject(std::string name)
{
    return new ParticleEmitterObject(name);
}
