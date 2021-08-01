#include "debug-module.h"

#include "imgui.h"

void DebugModule::RenderTimeSliceBoundaries(TimefieldRenderGraph& OutRenderGraph, Chart* const InSelectedChart, Time InTimeBegin, Time InTimeEnd) 
{
    if(!ShowTimeSliceBoundaries)
        return;

	InSelectedChart->IterateTimeSlicesInTimeRange(InTimeBegin, InTimeEnd, [&OutRenderGraph, this](TimeSlice InTimeSlice)
	{
        OutRenderGraph.SubmitTimefieldRenderCommand(0, InTimeSlice.TimePoint, [InTimeSlice, this](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
        {
		    const auto& timeFieldMetrics = InTimefieldMetrics;

            sf::RectangleShape line(sf::Vector2f(timeFieldMetrics.FieldWidth, 6));

		    line.setPosition(timeFieldMetrics.LeftSidePosition, InScreenY);
		    line.setFillColor(sf::Color(255, 0, 255, 255));
            
		    InRenderTarget->draw(line);

		    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
		    flags |= ImGuiWindowFlags_NoTitleBar;
		    flags |= ImGuiWindowFlags_NoResize;
		    flags |= ImGuiWindowFlags_AlwaysAutoResize;

		    ImGui::SetNextWindowPos({line.getPosition().x + timeFieldMetrics.FieldWidth + 6.f, line.getPosition().y});

		    ImGui::Begin(std::to_string(InTimeSlice.TimePoint).c_str(), &ShowTimeSliceBoundaries, flags);			
		    ImGui::Text(std::to_string(InTimeSlice.TimePoint).c_str());
		    ImGui::End();
        });
	});
	
}
