/***************************************************************************
# Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include "Falcor.h"

using namespace Falcor;

class ComparisonPass : public RenderPass
{
public:
    using SharedPtr = std::shared_ptr<ComparisonPass>;

    virtual Dictionary getScriptingDictionary() override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pContext, const RenderData& renderData) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;
    virtual void renderUI(Gui::Widgets& widget) override;

    void parseDictionary(const Dictionary& dict);

protected:
    ComparisonPass();
    virtual void createProgram() = 0;

    FullScreenPass::SharedPtr mpSplitShader;
    Texture::SharedPtr pLeftSrcTex;
    Texture::SharedPtr pRightSrcTex;
    Fbo::SharedPtr pDstFbo;
    Texture::SharedPtr mpArrowTex; ///< A texture storing a 16x16 grayscale arrow

    // Screen parameters
    bool mSwapSides = false; ///< Is the left input on the left side

    // Mouse parameters
    bool mMouseOverDivider = false; ///< Is the mouse over the divider?
    ivec2 mMousePos = ivec2(0, 0); ///< Where was mouse in last mouse event processed
    bool mDividerGrabbed = false; ///< Are we grabbing the divider?

    // Divider parameters
    float mSplitLoc = -1.0f; ///< Location of the divider as a fraction of screen width, values < 0 are initialized to 0.5
    uint32_t mDividerSize = 2; ///< Size of the divider (in pixels: 2*mDividerSize+1)
    bool mDrawArrows = false; ///< When hovering over divider, show arrows?

    // Label Parameters
    bool mShowLabels = false; ///< Show text labels for two images?
    bool mLabelsOnlyWhenHovering = false; ///< Show text always, or only when hovering over divider?
    std::string mLeftLabel = "Left side"; ///< Left label.  Set in Python script with "leftLabel"
    std::string mRightLabel = "Right side"; ///< Right label.  Set in Python script with "rightLabel"

    // Double-click detection Parameters
    Clock mClock; ///< Global clock used to track click times
    double mTimeOfLastClick = 0; ///< Time since mouse was last clicked
};
