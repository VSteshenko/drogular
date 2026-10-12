#pragma once

#include "domain/store_models.hpp"
#include "data/mini_store_time.hpp"

#include <chrono>
#include <vector>

struct MiniStoreDataset {
    std::vector<MiniStoreCategory> categories;
    std::vector<MiniStoreProduct> products;
    std::vector<MiniStoreOrder> orders;
};

inline MiniStoreDataset createMiniStoreDemoDataset() {
    return {
        {
            {
                1,
                {
                    {"en", "Coffee & Tea"},
                    {"de", "Kaffee & Tee"}
                },
                "coffee-tea"
            },
            {
                2,
                {
                    {"en", "Kitchen"},
                    {"de", "Küche"}
                },
                "kitchen"
            },
            {
                3,
                {
                    {"en", "Accessories"},
                    {"de", "Zubehör"}
                },
                "accessories"
            }
        },
        {
            {
                1,
                "COF-001",
                {
                    {"en", "House Blend Coffee"},
                    {"de", "Hausmischung Kaffee"}
                },
                {
                    {"en", "Balanced whole-bean coffee for everyday brewing."},
                    {"de", "Ausgewogener Bohnenkaffee für die tägliche Zubereitung."}
                },
                1,
                1290,
                18,
                true
            },
            {
                2,
                "TEA-001",
                {
                    {"en", "Earl Grey Tea"},
                    {"de", "Earl-Grey-Tee"}
                },
                {
                    {"en", "Black tea with bergamot in a reusable tin."},
                    {"de", "Schwarzer Tee mit Bergamotte in einer wiederverwendbaren Dose."}
                },
                1,
                890,
                24,
                true
            },
            {
                3,
                "KIT-001",
                {
                    {"en", "Glass Coffee Server"},
                    {"de", "Kaffeekanne aus Glas"}
                },
                {
                    {"en", "Heat-resistant server for pour-over coffee."},
                    {"de", "Hitzebeständige Kanne für handgefilterten Kaffee."}
                },
                2,
                2490,
                7,
                true
            },
            {
                4,
                "ACC-001",
                {
                    {"en", "Canvas Market Bag"},
                    {"de", "Einkaufstasche aus Canvas"}
                },
                {
                    {"en", "Reusable canvas bag for everyday shopping."},
                    {"de", "Wiederverwendbare Canvas-Tasche für den täglichen Einkauf."}
                },
                3,
                1590,
                12,
                true
            },
            {
                5,
                "KIT-002",
                {
                    {"en", "Digital Kitchen Scale"},
                    {"de", "Digitale Küchenwaage"}
                },
                {
                    {"en", "Compact scale with one-gram precision."},
                    {"de", "Kompakte Waage mit Ein-Gramm-Genauigkeit."}
                },
                2,
                2990,
                4,
                true
            },
            {
                6,
                "ACC-002",
                {
                    {"en", "Travel Tumbler"},
                    {"de", "Thermobecher"}
                },
                {
                    {"en", "Insulated stainless-steel tumbler."},
                    {"de", "Isolierter Trinkbecher aus Edelstahl."}
                },
                3,
                2190,
                0,
                true
            }
        },
        {
            {
                1008,
                {
                    "Mia Weber",
                    "mia@example.test"
                },
                {
                    {
                        2,
                        "Earl Grey Tea",
                        990,
                        1
                    }
                },
                990,
                MiniStoreOrderStatus::New,
                mini_store_time::ago(std::chrono::hours{2})
            },
            {
                1007,
                {
                    "Noah Fischer",
                    "noah@example.test"
                },
                {
                    {
                        4,
                        "Canvas Market Bag",
                        1590, 2
                    }
                },
                3180,
                MiniStoreOrderStatus::Processing,
                mini_store_time::ago(std::chrono::hours{12})
            },
            {
                1006,
                {
                    "Emma Klein",
                    "emma@example.test"
                },
                {
                    {
                        3,
                        "Glass Coffee Server",
                        2490,
                        1
                    },
                    {
                        1,
                        "House Blend Coffee",
                        1290,
                        1
                    }
                },
                3780,
                MiniStoreOrderStatus::Completed,
                mini_store_time::ago(std::chrono::hours{20})
            },
            {
                1005,
                {
                    "Liam Wagner",
                    "liam@example.test"
                },
                {
                    {
                        5,
                        "Digital Kitchen Scale",
                        2990,
                        2
                    }
                },
                5980,
                MiniStoreOrderStatus::New,
                mini_store_time::ago(std::chrono::hours{30})
            },
            {
                1004,
                {
                    "Sofia Becker",
                    "sofia@example.test"
                },
                {
                    {
                        6,
                        "Travel Tumbler",
                        2190,
                        1
                    }
                },
                2190,
                MiniStoreOrderStatus::Processing,
                mini_store_time::ago(std::chrono::hours{48})
            },
            {
                1003,
                {
                    "Ben Hoffmann",
                    "ben@example.test"
                },
                {
                    {
                        1,
                        "House Blend Coffee",
                        1290,
                        2
                    },
                    {
                        4,
                        "Canvas Market Bag",
                        1590,
                        1
                    }
                },
                4170,
                MiniStoreOrderStatus::Completed,
                mini_store_time::ago(std::chrono::hours{60})
            },
            {
                1002,
                {
                    "Lea Schulz",
                    "lea@example.test"
                },
                {
                    {
                        2,
                        "Earl Grey Tea",
                        990,
                        3
                    }
                },
                2970,
                MiniStoreOrderStatus::New,
                mini_store_time::ago(std::chrono::hours{72})
            },
            {
                1001,
                {
                    "Alex Morgan",
                    "alex@example.test"
                },
                {
                    {
                        1,
                        "House Blend Coffee",
                        1290,
                        2
                    },
                    {
                        4,
                        "Canvas Market Bag",
                        1590,
                        1
                    }
                },
                4170,
                MiniStoreOrderStatus::Processing,
                mini_store_time::ago(std::chrono::hours{96})
            },
            {
                1000,
                {
                    "Sam Taylor",
                    "sam@example.test"
                },
                {
                    {
                        3,
                        "Glass Coffee Server",
                        2490,
                        1
                    }
                },
                2490,
                MiniStoreOrderStatus::Completed,
                mini_store_time::ago(std::chrono::hours{120})
            }
        }
    };
}