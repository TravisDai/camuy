/* Copyright (c) 2019, 2020 Kevin Stehle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file        processing_element_left_border.h
 * @author      Kevin Stehle (stehle@stud.uni-heidelberg.de)
 * @date        2019-2020
 * @copyright   MIT License
 */

#ifndef PROCESSING_ELEMENT_LEFT_BORDER_H
#define PROCESSING_ELEMENT_LEFT_BORDER_H

#include <sstream>
#include <cassert>

#include "processing_element.h"
#include "activation_fifo.h"

template<typename WeightDatatype,
            typename ActivationDatatype,
            typename SumDatatype> class ProcessingElementLeftBorder: public ProcessingElement<WeightDatatype,
                                                                                                ActivationDatatype,
                                                                                                SumDatatype>
{

public:

    ProcessingElementLeftBorder(const PEPosition position,
                                const ProcessingElement<WeightDatatype,
                                                            ActivationDatatype,
                                                            SumDatatype>* const neighborUpperPtr,
                                ActivationFifo<ActivationDatatype>* const activationFifoPtr):
                                                                ProcessingElement<WeightDatatype,
                                                                                    ActivationDatatype,
                                                                                    SumDatatype>::ProcessingElement(position),
                                                                m_neighborUpperPtr{neighborUpperPtr},
                                                                m_activationFifoPtr{activationFifoPtr}
    {

        assert((ProcessingElement<WeightDatatype,
                                    ActivationDatatype,
                                    SumDatatype>::m_position.x) == 0);

//        std::cout << "PE (" << ProcessingElement<Datatype>::m_position.x << ", "
//                    << ProcessingElement<Datatype>::m_position.y << ") constructed with connection to activation FIFO";

//        if(ProcessingElement<Datatype>::m_position.y != 0)
//        {
//            std::cout << ", upper neighbor position (" << m_neighborUpperPtr->getPosition().x
//                                        << ", " << m_neighborUpperPtr->getPosition().y << ") ";
//        }

//        std::cout << std::endl;
    }

    void enableFifoInput(bool enabled)
    {
        m_fifoInputEnabledNext = enabled;
    }

    bool fifoInputEnabled() const
    {
        return m_fifoInputEnabledCurrent;
    }

    void setUpdateWeightSignal(const bool updateWeight)
    {
        ProcessingElement<WeightDatatype,
                            ActivationDatatype,
                            SumDatatype>::m_updateWeightNext = updateWeight;
    }

    void updateState() final
    {
        ProcessingElement<WeightDatatype,
                            ActivationDatatype,
                            SumDatatype>::updateState();

        m_fifoInputEnabledCurrent = m_fifoInputEnabledNext;
    }

    void readUpdateWeightSignals() final
    {
        if(m_neighborUpperPtr)
        {
            ProcessingElement<WeightDatatype,
                                ActivationDatatype,
                                SumDatatype>::m_updateWeightNext =
                                                        m_neighborUpperPtr->hasUpdateWeightSignal();
        }
    }

    void computeSum() final
    {

        if(m_fifoInputEnabledCurrent)
        {
            bool validSignalUpperNeighbor{true};

            if(m_neighborUpperPtr)
            {
                validSignalUpperNeighbor =  m_neighborUpperPtr->hasValidSignal();
            }

            if(validSignalUpperNeighbor)
            {

                ProcessingElement<WeightDatatype,
                                    ActivationDatatype,
                                    SumDatatype>::m_activationNext =
                                                            m_activationFifoPtr->pop();

                ProcessingElement<WeightDatatype,
                                    ActivationDatatype,
                                    SumDatatype>::m_sumNext =
                                        ProcessingElement<WeightDatatype,
                                                            ActivationDatatype,
                                                            SumDatatype>::m_activationNext*
                                        ProcessingElement<WeightDatatype,
                                                            ActivationDatatype,
                                                            SumDatatype>::loadWeight();

                ProcessingElement<WeightDatatype,
                                    ActivationDatatype,
                                    SumDatatype>::m_validNext = true;

                if(m_neighborUpperPtr)
                {
                    ProcessingElement<WeightDatatype,
                                        ActivationDatatype,
                                        SumDatatype>::m_sumNext += m_neighborUpperPtr->getSum();
                    ProcessingElement<WeightDatatype,
                                        ActivationDatatype,
                                        SumDatatype>::m_validNext &= validSignalUpperNeighbor;
                }

//                std::ostringstream outputStringStream;

//                outputStringStream << "PE (" << ProcessingElement<Datatype>::m_position.x << ", "
//                                    << ProcessingElement<Datatype>::m_position.y << ") weight: "
//                                    << ProcessingElement<Datatype>::loadWeight() << " input act: "
//                                    << ProcessingElement<Datatype>::m_activationNext;

//                if(m_neighborUpperPtr)
//                {
//                    outputStringStream << " input ps: " << m_neighborUpperPtr->getSum();
//                }

//                outputStringStream << " result: " << ProcessingElement<Datatype>::m_sumNext
//                                    << " valid signal: " << ProcessingElement<Datatype>::m_validNext
//                                    << " update weight signal: "
//                                    << ProcessingElement<Datatype>::m_updateWeightNext << std::endl;

//                std::cout << outputStringStream.str();
            }
        }
    }

private:

    const ProcessingElement<WeightDatatype,
                                ActivationDatatype,
                                SumDatatype>* const m_neighborUpperPtr{nullptr};

    ActivationFifo<ActivationDatatype>* const m_activationFifoPtr{nullptr};

    bool m_fifoInputEnabledCurrent{false};
    bool m_fifoInputEnabledNext{false};

};

#endif
