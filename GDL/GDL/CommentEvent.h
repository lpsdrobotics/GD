/**
 *  Game Develop
 *  2008-2010 Florian Rival (Florian.Rival@gmail.com)
 */

#ifndef COMMENTEVENT_H
#define COMMENTEVENT_H

#include <boost/shared_ptr.hpp>
#include "Event.h"
class TiXmlElement;

#if defined(GDE)
class Scene;
class Game;
class MainEditorCommand;
class wxWindow;
#endif

class CommentEvent : public BaseEvent
{
    public:
        CommentEvent() : BaseEvent(), r(255), v(230), b(109) {};
        virtual ~CommentEvent() {};
        virtual BaseEventSPtr Clone() { return boost::shared_ptr<BaseEvent>(new CommentEvent(*this));}

        void SaveToXml(TiXmlElement * eventElem) const;
        void LoadFromXml(const TiXmlElement * eventElem);

        int r;
        int v;
        int b;

        string com1;
        string com2;

#if defined(GDE)
        virtual void EditEvent(wxWindow* parent_, Game & game_, Scene & scene_, MainEditorCommand & mainEditorCommand_);
#endif

    private:

#if defined(GDE)
        virtual void RenderInBitmap() const;
        unsigned int CalculateNecessaryHeight() const;
#endif
};

#endif // COMMENTEVENT_H