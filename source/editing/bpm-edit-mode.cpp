#include "bpm-edit-mode.h"
#include "imgui.h"


bool BpmEditMode::OnMouseLeftButtonClicked(const bool InIsShiftDown) 
{
    if(static_Cursor.TimefieldSide != Cursor::FieldPosition::Middle)
        return false;

    if(_HoveredBpmPoint != nullptr)
    {
        _MovableBpmPoint = _HoveredBpmPoint;
        _MovableBpmPointInitialValue = *_MovableBpmPoint;

        static_Chart->RegisterTimeSliceHistory(_MovableBpmPoint->TimePoint);

        _PreviousBpmPoint = static_Chart->GetPreviousBpmPointFromTimePoint(_MovableBpmPoint->TimePoint);
        _NextBpmPoint = static_Chart->GetNextBpmPointFromTimePoint(_MovableBpmPoint->TimePoint);

        return false;
    }

    if(static_Flags.UseAutoTiming)
        PlaceAutoTimePoint();
    else
        PlaceTimePoint();


    return true;
}

bool BpmEditMode::OnMouseLeftButtonReleased() 
{
    if(_MovableBpmPoint != nullptr)
    {
        static_Chart->RevaluateBpmPoint(_MovableBpmPointInitialValue, *_MovableBpmPoint);

        _MovableBpmPoint = nullptr;
        return false;
    }

    return false;
}

bool BpmEditMode::OnMouseRightButtonClicked(const bool InIsShiftDown) 
{
    if(_HoveredBpmPoint && !_MovableBpmPoint)
    {
        if(static_Flags.UseAutoTiming)
        {
            if(BpmPoint* previousBpmPoint = static_Chart->GetPreviousBpmPointFromTimePoint(_HoveredBpmPoint->TimePoint))
            {
                if(BpmPoint* nextBpmPoint = static_Chart->GetNextBpmPointFromTimePoint(_HoveredBpmPoint->TimePoint))
                {
                    Time deltaTime = abs((previousBpmPoint->TimePoint) - nextBpmPoint->TimePoint);
	
                    double beatLength = double(deltaTime);
                    double newBpm = 60000.0 / beatLength;
    
                    previousBpmPoint->BeatLength = beatLength;
                    previousBpmPoint->Bpm = newBpm;
                }
            }
        }

        static_Chart->RemoveBpmPoint(*_HoveredBpmPoint);
        _HoveredBpmPoint = nullptr;

        _VisibleBpmPoints->clear();
    }

    return false;
}

void BpmEditMode::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd) 
{
    if(_PinnedBpmPoint)
    {
        InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(0, _PinnedBpmPoint->TimePoint, 
        [this](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
        {
            float posY = InRenderTarget->getView().getSize().y / 2.f;
            float posX = InTimefieldMetrics.LeftSidePosition + InTimefieldMetrics.FieldWidth + 64.f;

            sf::VertexArray line(sf::Lines, 2);
            line[0].position.x = InTimefieldMetrics.LeftSidePosition + InTimefieldMetrics.FieldWidth;
            line[0].position.y = InScreenY;
            line[0].color = sf::Color(255, 255, 255, 255);

            line[1].position.x = posX;
            line[1].position.y = posY;
            line[1].color = sf::Color(255, 255, 255, 255);

            InRenderTarget->draw(line);

            DisplayBpmNode(*_PinnedBpmPoint, posX , posY, true);
        });
    }

    //a bit hacky but if it works and is isolated it works for now I guess
	_VisibleBpmPoints = &(static_Chart->GetBpmPointsRelatedToTimeRange(InTimeBegin, InTimeEnd));

	for (auto& bpmPointPtr : *_VisibleBpmPoints)
	{
        InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(0, bpmPointPtr->TimePoint, 
        [this, bpmPointPtr](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
        {
            sf::RectangleShape bpmLine;
        
            if(_HoveredBpmPoint == bpmPointPtr)
            {
                bpmLine.setPosition(InTimefieldMetrics.LeftSidePosition, InScreenY - 8);
                bpmLine.setSize(sf::Vector2f(InTimefieldMetrics.FieldWidth, 16));
                bpmLine.setFillColor(sf::Color(128, 255, 128, 255));
            }
            else
            {
                bpmLine.setPosition(InTimefieldMetrics.LeftSidePosition, InScreenY - 2);
                bpmLine.setSize(sf::Vector2f(InTimefieldMetrics.FieldWidth, 4));
                bpmLine.setFillColor(sf::Color(255, 255, 255, 255));
            }

            InRenderTarget->draw(bpmLine);

            if(bpmPointPtr == _PinnedBpmPoint)
                return;

            sf::RectangleShape indicator;
            indicator.setPosition(InTimefieldMetrics.LeftSidePosition + InTimefieldMetrics.FieldWidth, InScreenY);
            indicator.setSize(sf::Vector2f(16, 1));
            indicator.setFillColor(sf::Color(255, 255, 255, 255));

            InRenderTarget->draw(indicator);

            //god is dead
            DisplayBpmNode(*bpmPointPtr, InTimefieldMetrics.LeftSidePosition + InTimefieldMetrics.FieldWidth + 8, InScreenY);
        });
    }

    if(static_Cursor.TimefieldSide != Cursor::FieldPosition::Middle || _HoveredBpmPoint != nullptr)
        return;

    InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(0, static_Cursor.UnsnappedTimePoint, 
    [this](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
    {
        sf::RectangleShape rectangle;
        
        rectangle.setPosition(InTimefieldMetrics.LeftSidePosition, InScreenY);
        rectangle.setSize(sf::Vector2f(InTimefieldMetrics.FieldWidth, 4));

        rectangle.setFillColor(sf::Color(255, 255, 255, 255));

        InRenderTarget->draw(rectangle);
    });
}

void BpmEditMode::Tick() 
{
    if(_MovableBpmPoint)
    {
        _MovableBpmPoint->TimePoint = static_Cursor.UnsnappedTimePoint;

        if(!static_Flags.UseAutoTiming)
            return;

        if(_PreviousBpmPoint)
        {
            Time deltaTime = abs((_PreviousBpmPoint->TimePoint) - _MovableBpmPoint->TimePoint);
	
            double beatLength = double(deltaTime);
            double newBpm = 60000.0 / beatLength;
    
            _PreviousBpmPoint->BeatLength = beatLength;
            _PreviousBpmPoint->Bpm = newBpm;
        }

        if(_NextBpmPoint)
        {
            Time deltaTime = abs(_NextBpmPoint->TimePoint - _MovableBpmPoint->TimePoint);
	
            double beatLength = double(deltaTime);
            double newBpm = 60000.0 / beatLength;
    
            _MovableBpmPoint->BeatLength = beatLength;
            _MovableBpmPoint->Bpm = newBpm;
        }

        return;
    }

    for (auto& bpmPointPtr : *_VisibleBpmPoints)
	{
        if(abs(static_Cursor.UnsnappedTimePoint - bpmPointPtr->TimePoint) < 20 && static_Cursor.TimefieldSide == Cursor::FieldPosition::Middle)
            return void(_HoveredBpmPoint = bpmPointPtr);
    }

    _HoveredBpmPoint = nullptr;
}

void BpmEditMode::PlaceAutoTimePoint() 
{
    Time cursorTime = static_Cursor.UnsnappedTimePoint;
    BpmPoint* placedBpmPoint = nullptr;

    if(BpmPoint* previousBpmPoint = static_Chart->GetPreviousBpmPointFromTimePoint(static_Cursor.UnsnappedTimePoint))
    {
        Time deltaTime = abs(previousBpmPoint->TimePoint - cursorTime);
	
        double beatLength = double(deltaTime);
        double newBpm = 60000.0 / beatLength;
    
        previousBpmPoint->BeatLength = beatLength;
        previousBpmPoint->Bpm = newBpm;
    }

    if(BpmPoint* nextBpmPoint =  static_Chart->GetNextBpmPointFromTimePoint(cursorTime))
    {
        Time deltaTime = abs(nextBpmPoint->TimePoint - cursorTime);
	
        double beatLength = double(deltaTime);
        double newBpm = 60000.0 / beatLength;
    
        static_Chart->PlaceBpmPoint(cursorTime, newBpm, beatLength);
    }
    else   
        static_Chart->PlaceBpmPoint(cursorTime, 120.0, 60000.0 / 120.0);
}

void BpmEditMode::PlaceTimePoint() 
{
    BpmPoint* previousBpmPoint = static_Chart->GetPreviousBpmPointFromTimePoint(static_Cursor.UnsnappedTimePoint);

    if(previousBpmPoint)
        static_Chart->PlaceBpmPoint(static_Cursor.UnsnappedTimePoint, previousBpmPoint->Bpm, previousBpmPoint->BeatLength);
    else
        static_Chart->PlaceBpmPoint(static_Cursor.UnsnappedTimePoint, 120.0, 60000.0 / 120.0);
}


void BpmEditMode::DisplayBpmNode(BpmPoint& InBpmPoint, const int InScreenX, const int InScreenY, const bool InIsPinned) 
{
    ImGuiWindowFlags windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_NoTitleBar;
	windowFlags |= ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoResize;
	windowFlags |= ImGuiWindowFlags_NoCollapse;
	windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	windowFlags |= ImGuiWindowFlags_NoScrollbar;

	bool open = true;

	ImGui::SetNextWindowPos({ float(InScreenX), float(InScreenY) });
	ImGui::Begin(std::to_string((int)&InBpmPoint).c_str(), &open, windowFlags);    

    if(InIsPinned)
    {
        if(ImGui::Button("Unpin"))
        {
            _PinnedBpmPoint = nullptr;
        }
    }
    else
    {
	    if(ImGui::Button("Pin"))
        {
            _PinnedBpmPoint = &InBpmPoint;
        }        
    }

	ImGui::SameLine();

	ImGui::Text("BPM");
	ImGui::SameLine();
	ImGui::PushItemWidth(96);

    float bpmFloat = float(InBpmPoint.Bpm);
	ImGui::DragFloat(" ", &bpmFloat, 0.1f, 0.01f, 2000.0f);
	InBpmPoint.Bpm = double(bpmFloat);
    InBpmPoint.BeatLength = 60000.0 / InBpmPoint.Bpm;
    
    ImGui::SameLine();
	ImGui::End();
}