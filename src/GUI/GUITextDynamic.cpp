//
// Created by engin on 14.11.2017.
//

#include <SDL_timer.h>
#include "GUITextDynamic.h"
#include "../Utils/GLMUtils.h"

void GUITextDynamic::render() {
    float totalAdvance = 0.0f;

    renderProgram->setUniform("inColor", color);

    renderProgram->setUniform("orthogonalProjectionMatrix", glHelper->getOrthogonalProjectionMatrix());

    glm::mat4 currentTransform;

    //Setup position
    float quadPositionX, quadPositionY, quadSizeX, quadSizeY;
    const Glyph *glyph;
    if(textList.size() == 0) {
        return;
    }
    int lineCount=1;//the line 0 would be out of the box

    int MaxLineCount = (height)/(lineHeight);
    int removeLineCount = textList.size() + totalExtraLines - MaxLineCount;
    if(removeLineCount > 0) {
        std::list<TimedText>::iterator it = textList.begin();
        std::advance(it,removeLineCount);
        for(std::list<TimedText>::iterator lineIt = textList.begin(); lineIt != it; lineIt++) {
            totalExtraLines = totalExtraLines - lineIt->extraLines;
        }
        textList.erase(textList.begin(),it);
    }

    for(std::list<TimedText>::iterator lineIt = textList.begin(); lineIt != textList.end(); lineIt++, lineCount++) {
        if(SDL_GetTicks() - lineIt->time > duration) {
            std::list<TimedText>::iterator test = lineIt;
            lineIt++;
            totalExtraLines = totalExtraLines - test->extraLines;
            textList.erase(test);
        } else {
            for (int character = 0; character < lineIt->text.length(); ++character) {
                glm::vec3 lineTranslate = translate + glm::vec3(0, height - (lineCount * lineHeight), 0);

                glyph = face->getGlyph(lineIt->text.at(character));
                quadSizeX = glyph->getSize().x / 2.0f;
                quadSizeY = glyph->getSize().y / 2.0f;

                quadPositionX = totalAdvance + glyph->getBearing().x + quadSizeX; //origin is left side
                quadPositionY = glyph->getBearing().y - quadSizeY; // origin is the bottom line


                /**
                 * the scale, translate and rotate functions apply the transition to first element, so the below code is
                 * scale to quadSizeX/Y * this->scale first,
                 * than translate to quadPositionX/Y - width/height* scale/2,
                 * than rotate using orientation,
                 * than translate to this->translate
                 *
                 * The double translate is because we want to rotate from center of the text.
                 */
                if (isRotated) {
                    currentTransform = glm::scale(
                            glm::translate(
                                    (glm::translate(glm::mat4(1.0f), lineTranslate) * glm::mat4_cast(orientation)),
                                    glm::vec3(quadPositionX, quadPositionY, 0) -
                                    glm::vec3(width * scale.x / 2.0f, height * scale.y / 2.0f, 0.0f)),
                            this->scale * glm::vec3(quadSizeX, quadSizeY, 1.0f)
                    );
                } else {
                    //this branch removes quaternion cast, so double translate is not necessary.
                    currentTransform = glm::scale(
                            glm::translate(glm::mat4(1.0f), lineTranslate +
                                                            glm::vec3(quadPositionX, quadPositionY, 0) -
                                                            glm::vec3(width * scale.x / 2.0f, height * scale.y / 2.0f,
                                                                      0.0f)),
                            this->scale * glm::vec3(quadSizeX, quadSizeY, 1.0f)
                    );
                }

                if (!renderProgram->setUniform("worldTransformMatrix", currentTransform)) {
                    std::cerr << "failed to set uniform \"worldTransformMatrix\"" << std::endl;
                }

                if (!renderProgram->setUniform("GUISampler", glyphAttachPoint)) {
                    std::cerr << "failed to set uniform \"GUISampler\"" << std::endl;
                }
                glHelper->attachTexture(glyph->getTextureID(), glyphAttachPoint);
                glHelper->render(renderProgram->getID(), vao, ebo, (const GLuint) (faces.size() * 3));

                totalAdvance += glyph->getAdvance() / 64;
                if(totalAdvance + maxCharWidth >= width) {
                    lineCount++;
                    totalAdvance = 0;
                    if(!lineIt->renderedBefore) {
                        lineIt->extraLines++;
                        totalExtraLines++;
                    }
                }
            }
            lineIt->renderedBefore = true;
            totalAdvance = 0;
        }

    }

}