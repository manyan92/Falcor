/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
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
#include "Framework.h"
#include "SceneRenderPass.h"

namespace Falcor
{
    static std::string kColor = "color";
    static std::string kDepth = "depth";

    static SceneRenderPass::PassData createRenderPassData()
    {
        RenderPass::PassData data;
        RenderPass::PassData::Field color;
        color.bindFlags = Resource::BindFlags::RenderTarget;
        color.name = kColor;
        color.pType = ReflectionResourceType::create(ReflectionResourceType::Type::Texture, ReflectionResourceType::Dimensions::Texture2D, ReflectionResourceType::StructuredType::Invalid, ReflectionResourceType::ReturnType::Unknown, ReflectionResourceType::ShaderAccess::Read);
        data.outputs.push_back(color);

        RenderPass::PassData::Field depth;
        depth.name = kDepth;
        depth.required = false;
        depth.format = ResourceFormat::Unknown;
        depth.bindFlags = Resource::BindFlags::DepthStencil;
        data.inputs.push_back(depth);

        return data;
    }

    const SceneRenderPass::PassData SceneRenderPass::kRenderPassData = createRenderPassData();

    SceneRenderPass::SharedPtr SceneRenderPass::create()
    {
        try
        {
            return SharedPtr(new SceneRenderPass);
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }

    SceneRenderPass::SceneRenderPass() : RenderPass("SceneRenderPass", nullptr)
    {
        GraphicsProgram::SharedPtr pProgram = GraphicsProgram::createFromFile("RenderPasses/SceneRenderPass.slang", "", "ps");
        mpState = GraphicsState::create();
        mpState->setProgram(pProgram);
        mpVars = GraphicsVars::create(pProgram->getReflector());
        mpFbo = Fbo::create();
        
        DepthStencilState::Desc dsDesc;
        dsDesc.setDepthTest(true).setDepthWriteMask(false).setStencilTest(false).setDepthFunc(DepthStencilState::Func::LessEqual);
        mpDsNoDepthWrite = DepthStencilState::create(dsDesc);
    }

    void SceneRenderPass::sceneChangedCB()
    {
        mpSceneRenderer = nullptr;
        if (mpScene)
        {
            mpSceneRenderer = SceneRenderer::create(mpScene);
        }
    }

    bool SceneRenderPass::isValid(std::string& log)
    {
        bool b = true;
        if (mpSceneRenderer == nullptr)
        {
            log += "SceneRenderPass must have a scene attached to it\n";
            b = false;
        }

        const auto& pColor = mpFbo->getColorTexture(0).get();
        if (!pColor)
        {
            log += "SceneRenderPass must have a color texture attached\n";
            b = false;
        }

        if (mpFbo->checkStatus() == false)
        {
            log += "SceneRenderPass FBO is invalid, probably because the depth and color textures have different dimensions";
            b = false;
        }

        return b;
    }

    bool SceneRenderPass::setInput(const std::string& name, const std::shared_ptr<Resource>& pResource)
    {
        if (name == kDepth)
        {
            Texture::SharedPtr pDepth = std::dynamic_pointer_cast<Texture>(pResource);
            mpFbo->attachDepthStencilTarget(pDepth);
            if (pDepth)
            {
                mpState->setDepthStencilState(mpDsNoDepthWrite);
                mClearFlags = FboAttachmentType::Color;
            }
            else
            {
                mpState->setDepthStencilState(nullptr);
                mClearFlags = FboAttachmentType::Color | FboAttachmentType::Depth;
            }
        }
        else
        {
            logError("SceneRenderPass::setInput() - trying to set `" + name + "` which doesn't exist in this render-pass");
            return false;
        }
        return false;
    }

    bool SceneRenderPass::setOutput(const std::string& name, const std::shared_ptr<Resource>& pResource)
    {
        if (!mpFbo)
        {
            logError("SceneRenderPass::setOutput() - please call onResizeSwapChain() before setting an input");
            return false;
        }

        if (name == kColor)
        {
            Texture::SharedPtr pColor = std::dynamic_pointer_cast<Texture>(pResource);
            mpFbo->attachColorTarget(pColor, 0);
        }
        else
        {
            logError("SceneRenderPass::setOutput() - trying to set `" + name + "` which doesn't exist in this render-pass");
            return false;
        }

        return true;
    }

    void SceneRenderPass::execute(RenderContext* pContext)
    {
        if (mpFbo->getDepthStencilTexture() == nullptr)
        {
            Texture::SharedPtr pDepth = Texture::create2D(mpFbo->getWidth(), mpFbo->getHeight(), ResourceFormat::D32Float, 1, 1, nullptr, Resource::BindFlags::DepthStencil);
            mpFbo->attachDepthStencilTarget(pDepth);
        }

        pContext->clearFbo(mpFbo.get(), mClearColor, 1, 0, mClearFlags);

        if (mpSceneRenderer)
        {
            mpState->setFbo(mpFbo);
            pContext->pushGraphicsState(mpState);
            pContext->pushGraphicsVars(mpVars);
            mpSceneRenderer->renderScene(pContext);
            pContext->popGraphicsState();
            pContext->popGraphicsVars();
        }
    }

    std::shared_ptr<Resource> SceneRenderPass::getOutput(const std::string& name) const
    {
        if (name == kColor)
        {
            return mpFbo->getColorTexture(0);
        }        
        else return RenderPass::getOutput(name);
    }

    std::shared_ptr<Resource> SceneRenderPass::getInput(const std::string& name) const
    {
        if (name == kDepth)
        {
            return mpFbo->getDepthStencilTexture();
        }
        else return RenderPass::getInput(name);
    }

    void SceneRenderPass::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
    {
        pGui->addRgbaColor("Clear color", mClearColor);
    }
}