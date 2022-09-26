/*
 * Assembly-language implementations of the drawing functions
 */

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/* Offsets of struct Rect fields */
#define RECT_X_OFFSET        0
#define RECT_Y_OFFSET        4
#define RECT_WIDTH_OFFSET    8
#define RECT_HEIGHT_OFFSET   12

	.section .text

/***********************************************************************
   Helper functions
   You should declare these as .globl so that
   they can be called from the unit test code.
 ***********************************************************************/

/*
 * Determine whether or not a specified pixel represented by (x,y) 
 * falls within the width and height range of an image
 * 
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of the pixel
 *   %edx - y coordinate of the pixel
 * 
 * Returns (in %rax):
 * 	1 if not in-bounds, 0 if in-bounds
 */
	.globl in_bounds
in_bounds:
	// TODO: Code goes here
        ret

/*
 * Converts a pixel coordinate represented by (x,y) to an index
 * within an array pointed to by "data" within the image struct.
 * Does not get called if in_bounds returns 1.
 * 
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of the pixel
 *   %edx - y coordinate of the pixel
 * 
 * Returns (in %eax):
 * 	the index within the array pointed to by "data"
 * 	that would contain the same information as the
 * 	specified (x,y) coordinate within said image
 */
	.globl compute_index
compute_index:
	movl IMAGE_WIDTH_OFFSET(%rdi), %r10d	// store img->width in r10d
	imull %r10d, %edx			// store (width * y) in edx
	addl %edx, %esi				// store (width * y) + x in esi
	movl %esi, %eax				// store total in rax
        ret

/*
 * Constrains a value within the specified maximum and minimum
 * by making the value min if it's less than minimum value and
 * making the value max if it's more than the maximum value.
 * 
 * Parameters:
 *   %rdi - value to be clamped if necessary
 *   %esi - minimum value
 *   %edx - maximum value
 * 
 * Returns (in %eax):
 * 	the new clamped value
 */
	.globl clamp
clamp:
	movl %rdi, %eax		// move the value to eax
        cmpl %esi, %eax		// compare min to value
        jge .L2			// if esi >= eax, then jump to L2
        movl %esi, %eax		// otherwise, store min in eax
        jmp .L4			// jump to L4
.L2:
        movl %rdi, %eax		// move the value to eax again
        cmpl %edx, %eax		// compare max to value
        jle .L3			// if edx <= eax, then jump to L3
        movl %edx, %eax		// otherwise, store max in eax
	jmp .L4			// jump to L4
.L3:
        movl %rdi, %eax		// move the value to eax to return value
.L4:
        ret

/*
 * Returns the red part of the color, or the 8 left-most bits
 * 
 * Parameters:
 *   %rdi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %rax):
 * 	the red color value
 */
	.globl get_r
get_r:
	// TODO: Code goes here
        ret

/*
 * Returns the green part of the color, or the 8 middle-left bits
 * 
 * Parameters:
 *   %rdi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %rax):
 * 	the green color value
 */
	.globl get_g
get_g:
	// TODO: Code goes here
        ret

/*
 * Returns the blue part of the color, or the 8 middle-right bits
 * 
 * Parameters:
 *   %rdi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %rax):
 * 	the blue color value
 */
	.globl get_b
get_b:
	// TODO: Code goes here
        ret

/*
 * Returns the alpha part of the color, or the 8 right-most bits
 * 
 * Parameters:
 *   %rdi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %rax):
 * 	the alpha value
 */
	.globl get_a
get_a:
	// TODO: Code goes here
        ret

/*
 * Calculates the value of an individual color component in a
 * foreground image being blended with the value of the same
 * individual color component in a background image.
 * 
 * Parameters:
 *   %esi - color component value in the foreground image
 *   %edx - color component value in the background image
 *   %ecx - alpha value to be used in blending calculation
 * 
 * Returns (in %rax):
 * 	the component-blended value
 */
	.globl blend_components
blend_components:
	// TODO: Code goes here
        ret

/*
 * Uses all the blended values for each color component and
 * combines it into one wholistic color value by using the
 * 4 8-bit color component values to make one 32-bit color.
 * Sets alpha value to 255 for total opacity.
 * 
 * Parameters:
 *   %esi - color component value in the foreground image
 *   %edx - color component value in the background image
 * 
 * Returns (in %rax):
 * 	the color-blended value
 */
	.globl blend_colors
blend_colors:
	// TODO: Code goes here
        ret

/*
 * Modifies a pixel in the background image to represent
 * the blended color value of the corresponding pixels in
 * the background image and the foreground image.
 * 
 * Parameters:
 *   %rdi     - pointer to the struct Image
 *   %rsi     - the index of the pixel to modify
 *   %ecx     - uint32_t color value
 */
	.globl set_pixel
set_pixel:
	// TODO: Code goes here
        ret

/*
 * Square an int64_t value.
 *
 * Parameters:
 *   %rdi     - an int64_t value
 * 
 * Returns (in %rax):
 * 	the square of the value
 */
	.globl square
square:
        imulq %rdi, %rdi
	movq %rdi, %rax
        ret

/*
 * Find the square of the distance between two pixels.
 * Coordinates are int4_t values.
 *
 * Parameters:
 *   %rdi     - x1
 *   %rsi     - y1
 *   %rdx     - x2
 *   %rcx     - y2
 * 
 * Returns (in %rax):
 * 	the square of the distance between
 * 	the coordinates (x1, y1) and (x2, y2)
 */
	.globl square_dist
square_dist:
	// save the values of callee-saved registers
	pushq %r12
	pushq %r13
	subq $8, %rsp 	 // aligns the stack pointer
	
	movq %rsi, %r12  // save the value of y1 in r12
	movq %rcx, %r13  // save the value of y2 in r13
	
	subq %rdi, %rdx  // store x2-x1 in rdx
	movq %rdx, %rdi  // prepare to call square
	call square	 // store (x2-x1)^2 in rax
	
	subq %r12, %r13  // store y1-y2 in r13
	movq %rax, %r12  // store (x2-x1)^2 in r12 now
	movq %r13, %rdi  // prepare to call square
	call square	 // store (y2-y1)^2 in rax
	
	addq %r12, %rax	 // store the square_dist in rax
	
	// restore the values of callee-saved registers
	addq $8, %rsp 	 // restore the stack pointer
	popq %r12
	popq %r13
        ret



/***********************************************************************
   Public API functions
 ***********************************************************************/

/*
 * Draw a pixel.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate (pixel column)
 *   %edx     - y coordinate (pixel row)
 *   %ecx     - uint32_t color value
 */
	.globl draw_pixel
draw_pixel:
	/* TODO: implement */
	ret


/*
 * Draw a rectangle.
 * The rectangle has rect->x,rect->y as its upper left corner,
 * is rect->width pixels wide, and rect->height pixels high.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %rsi     - pointer to struct Rect
 *   %edx     - uint32_t color value
 */
	.globl draw_rect
draw_rect:
	/* TODO: implement */
	ret

/*
 * Draw a circle.
 * The circle has x,y as its center and has r as its radius.
 *
 * Parameters:
 *   %rdi     - pointer to struct Image
 *   %esi     - x coordinate of circle's center
 *   %edx     - y coordinate of circle's center
 *   %ecx     - radius of circle
 *   %r8d     - uint32_t color value
 */
	.globl draw_circle
draw_circle:
	/* TODO: implement */
	ret

/*
 * Draw a tile by copying all pixels in the region
 * enclosed by the tile parameter in the tilemap image
 * to the specified x/y coordinates of the destination image.
 * No blending of the tile pixel colors with the background
 * colors should be done.
 *
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of location where tile should be copied
 *   %edx - y coordinate of location where tile should be copied
 *   %rcx - pointer to Image (the tilemap)
 *   %r8  - pointer to Rect (the tile)
 */
	.globl draw_tile
draw_tile:
	/* TODO: implement */
	ret

/*
 * Draw a sprite by copying all pixels in the region
 * enclosed by the sprite parameter in the spritemap image
 * to the specified x/y coordinates of the destination image.
 * The alpha values of the sprite pixels should be used to
 * blend the sprite pixel colors with the background
 * pixel colors.
 *
 * Parameters:
 *   %rdi - pointer to Image (dest image)
 *   %esi - x coordinate of location where sprite should be copied
 *   %edx - y coordinate of location where sprite should be copied
 *   %rcx - pointer to Image (the spritemap)
 *   %r8  - pointer to Rect (the sprite)
 */
	.globl draw_sprite
draw_sprite:
	/* TODO: implement */
	ret

/*
vim:ft=gas:
*/
