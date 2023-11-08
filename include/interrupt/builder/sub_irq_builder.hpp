#pragma once

#include <flow/impl.hpp>
#include <interrupt/impl/sub_irq_impl.hpp>

#include <type_traits>

namespace interrupt {
/**
 * Declare a sub-interrupt under a shared interrupt.
 *
 * This object is designed only to live in a constexpr context. The template
 * specialization should be declared by the user while the interrupt::Manager
 * creates and manages instances of shared_irq.
 */
template <typename Config> struct sub_irq_builder {
    using irq_callback_t = typename Config::irq_callback_t;

  private:
    irq_callback_t interrupt_service_routine;

  public:
    /**
     * Add interrupt service routine(s) to be executed when this IRQ is
     * triggered.
     *
     * This should be used only by interrupt::Manager.
     *
     * @param flow_description
     *      See flow::Builder<>.add()
     */
    template <typename IrqType>
    constexpr void add(auto const &flow_description) {
        if constexpr (std::is_same_v<irq_callback_t, IrqType>) {
            interrupt_service_routine.add(flow_description);
        }
    }

    /**
     * @return sub_irq::impl specialization optimized for size and runtime.
     */
    template <typename BuilderValue>
    [[nodiscard]] constexpr auto build() const {
        constexpr auto run_flow = [] {
            auto constexpr flow_builder =
                BuilderValue::value.interrupt_service_routine;
            auto constexpr flow_size = flow_builder.size();
            auto constexpr flow =
                flow_builder.template topo_sort<flow::impl, flow_size>();
            flow.value()();
        };

        constexpr auto flow_builder =
            BuilderValue::value.interrupt_service_routine;
        constexpr auto flow_size = flow_builder.size();
        auto const optimized_irq_impl =
            sub_irq_impl<Config, flow::impl<irq_callback_t::name, flow_size>>(
                run_flow);

        return optimized_irq_impl;
    }
};
} // namespace interrupt
