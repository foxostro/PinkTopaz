#import <XCTest/XCTest.h>
#import <glm/glm.hpp>
#import "Morton.hpp"

using namespace glm;

@interface MortonTests : XCTestCase

@end

@implementation MortonTests

- (void)testZero {
    const ivec3 zero(0, 0, 0);
    const Morton3 morton(zero);
    const uint32_t code = (uint32_t)morton;
    XCTAssertEqual(code, 0);
}

- (void)testEncode0xF {
    // Interleaving bits of the morton code gives us back the coordinates.
    const ivec3 point(0b1111, 0b1111, 0b1111);
    const Morton3 morton(point);
    const uint32_t code = (uint32_t)morton;
    XCTAssertEqual(code, 0b111111111111);
}

- (void)testEncode0xA {
    const ivec3 point(0b1110, 0b1110, 0b1110);
    const Morton3 morton(point);
    const uint32_t code = (uint32_t)morton;
    XCTAssertEqual(code, 0b111111111000);
}

- (void)testDecode {
    // De-interleaving bits of the morton code gives us back the coordinates.
    XCTAssertEqual(ivec3(0b1110, 0b1110, 0b1110),
                   Morton3::decode(0b111111111000));
    XCTAssertEqual(ivec3(0b1010, 0b1111, 0b1111),
                   Morton3::decode(0b111110111110));
}

- (void)testIncDec {
    const Morton3 a(ivec3(10, 11, 12));
    Morton3 b(a);
    
    b.incX();
    XCTAssertEqual(b.decode(), ivec3(11, 11, 12));
    
    b.incY();
    XCTAssertEqual(b.decode(), ivec3(11, 12, 12));
    
    b.incZ();
    XCTAssertEqual(b.decode(), ivec3(11, 12, 13));
    
    b.decX();
    XCTAssertEqual(b.decode(), ivec3(10, 12, 13));
    
    b.decY();
    XCTAssertEqual(b.decode(), ivec3(10, 11, 13));
    
    b.decZ();
    XCTAssertEqual(b.decode(), ivec3(10, 11, 12));
}

@end
