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
 * Returns (in %eax):
 * 	1 if not in-bounds, 0 if in-bounds
 */
	.globl in_bounds
in_bounds:	
	movl IMAGE_WIDTH_OFFSET(%rdi), %r11d		//store img->width in %r11d
	movl IMAGE_HEIGHT_OFFSET(%rdi), %r12d		//store img->height in %r12d
	cmpl %r11d, %esi                            //compare x and image->width
	jge .LoutOfBounds                           //if x too high, jump to LoutOfBounds
	cmpl %r12d, %edx	                        //compare y and image->width
	jge .LoutOfBounds                           //if y too high, jump to LoutOfBounds
	cmpl $0, %esi                               //compare x and 0
	jl .LoutOfBounds                            //if x too low, jump to LoutOfBounds
	cmpl $0, %edx                               //compare y and 0
	jl .LoutOfBounds                            //if y too low, jump to LoutOfBounds
	mov $0, %eax								//set return value to 0
	jmp .Ldone

.LoutOfBounds:
	mov $1, %eax                                //set return value to 1

.Ldone:
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
	movl IMAGE_WIDTH_OFFSET(%rdi), %r10d		// store img->width in r10d
	movl %edx, %eax					// store y in eax
	mull %r10d					// store (width * y) in eax
	addl %eax, %esi					// store (width * y) + x in esi
	movl %esi, %eax					// store total in rax
	ret

/*
 * Constrains a value within the specified maximum and minimum
 * by making the value min if it's less than minimum value and
 * making the value max if it's more than the maximum value.
 * 
 * Parameters:
 *   %edi - value to be clamped if necessary
 *   %esi - minimum value
 *   %edx - maximum value
 * 
 * Returns (in %eax):
 * 	the new clamped value
 */
	.globl clamp
clamp:
	movl %edi, %eax		// move the value to eax
    cmpl %esi, %eax		// compare min to value
    jge .Lgreater		// if esi >= eax, then jump to Lgreater
    movl %esi, %eax		// otherwise, store min in eax
    jmp .Lclampend	    	// jump to Lclampend
.Lgreater:
   	movl %edi, %eax		// move the value to eax again
	cmpl %edx, %eax		// compare max to value
    jle .Lless			// if edx <= eax, then jump to Lless
    movl %edx, %eax		// otherwise, store max in eax
    jmp .Lclampend	    	// jump to Lclampend
.Lless:
    movl %edi, %eax		// move the value to eax to return value
.Lclampend:
    ret

/*
 * Returns the red part of the color, or the 8 left-most bits
 * 
 * Parameters:
 *   %edi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %al):
 * 	the red color value
 */
	.globl get_r
get_r:
	shrl $24, %edi			//move the 8 left-most bits of edi to the right
	andl $255, %edi			//isolate the last 8 bits of edi
	movb %dil, %al			//move last 8 bits of edi into al
	ret

/*
 * Returns the green part of the color, or the 8 middle-left bits
 * 
 * Parameters:
 *   %edi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %al):
 * 	the green color value
 */
	.globl get_g
get_g:
	shrl $16, %edi			//move the 8 middle-left bits of edi to the right
	andl $255, %edi			//isolate the last 8 bits of edi
	movb %dil, %al			//move last 8 bits of edi into al
	ret

/*
 * Returns the blue part of the color, or the 8 middle-right bits
 * 
 * Parameters:
 *   %edi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %eax):
 * 	the blue color value
 */
	.globl get_b
get_b:
	shrl $8, %edi			//move the 8 middle-right bits of edi to the right
	andl $255, %edi			//isolate the last 8 bits of edi
	movb %dil, %al			//move last 8 bits of edi into al
	ret

/*
 * Returns the alpha part of the color, or the 8 right-most bits
 * 
 * Parameters:
 *   %edi - the color value with r, b, g, and alpha information
 * 
 * Returns (in %al):
 * 	the alpha value
 */
	.globl get_a
get_a:
	andl $255, %edi			//isolate last 8 bits of edi
	movb %dil, %al			//move last 8 bits of edi into al
	ret

/*
 * Calculates the value of an individual color component in a
 * foreground image being blended with the value of the same
 * individual color component in a background image.
 * 
 * Parameters:
 *   %edi - color component value in the foreground image
 *   %esi - color component value in the background image
 *   %edx - alpha value to be used in blending calculation
 * 
 * Returns (in %al):
 * 	the component-blended value
 */
	.globl blend_components
blend_components:
	pushq %r12
	movl %edx, %r11d			//copy the alpha value into r11d
	movl $255, %r12d			//copy 255 into r12d
	subl %r11d, %r12d			//subtract alpha from 255 and store in r12d
	movl %esi, %eax				//move background color into eax
	imull %r12d, %eax			//multiply (255-alpha) by background color and store in eax
	movl %eax, %r12d			//move (255-alpha)*bg into r12d
	movl %edi, %eax				//move foreground color into eax
	imull %edx, %eax			//multiply alpha by foreground color and store in eax
	addl %r12d, %eax			//add (255-alpha)*bg to (alpha*fg) and store in eax
	movl $255, %edi				//copy 255 into edi
	movl $0, %edx				//prepare for division
	divl %edi	  	 	  	//divide alpha*fg+(255 - alpha)*bg by 255 and store in eax
	popq %r12
	ret					//return the quotient

/*
 * Uses all the blended values for each color component and
 * combines it into one wholistic color value by using the
 * 4 8-bit color component values to make one 32-bit color.
 * Sets alpha value to 255 for total opacity.
 * 
 * Parameters:
 *   %edi - color component value in the foreground image
 *   %esi - color component value in the background image
 * 
 * Returns (in %eax):
 * 	the color-blended value
 */
	.globl blend_colors
blend_colors:
	// save the values of callee-saved registers in the stack
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	subq $8, %rsp

	movl %edi, %r8d			//save foreground value for later
	movl %esi, %r9d			//save background value for later

	/*use r11, r12, and rcx to as parameters for blend_components*/
	
	/*get r component of the foreground*/
	shrl $24, %r8d			//move the 8 left-most bits of foreground to the right, store in r8d
	andl $255, %r8d			//isolate the last 8 bits of r8d, store in r8d
	movl %r8d, %r11d		//move red value in r8d into r11d
	movl %edi, %r8d			//save foreground value for later

	/*get r value of the background*/
	shrl $24, %r9d			//move the 8 left-most bits of background to the right, store in r9d
	andl $255, %r9d			//isolate the last 8 bits of r9d, store in r9d
	movl %r9d, %r12d		//move red value in r9d into r12d
	movl %esi, %r9d			//save background value for later

	/*get a value of the foreground*/
	andl $255, %r8d			//isolate last 8 bits of foreground, store in r8d
	movl %r8d, %ecx			//move alpha value in r8d into ecx
	movl %edi, %r8d			//save foreground value for later

	/*call blend_components*/
	movl %r11d, %edi		//prepare to call blend_components
	movl %r12d, %esi		//prepare to call blend_components
	movl %ecx, %edx			//prepare to call blend_components
	call blend_components	//store blended r value in al
	movb %al, %r13b			//move blended r value to r13b, which is callee-saved
	movl %r8d, %edi			//move foreground color back into edi
	movl %r9d, %esi			//move background color back into esi

	/*get g component of the foreground*/
	shrl $16, %r8d			//move the 8 middle-left bits of foreground to the right, store in r8d
	andl $255, %r8d			//isolate the last 8 bits of r8d, store in r8d
	movl %r8d, %r11d		//move green value in r8d into r11d
	movl %edi, %r8d			//save foreground value for later

	/*get g value of the background*/
	shrl $16, %r9d			//move the 8 middle-left bits of background to the right, store in r9d
	andl $255, %r9d			//isolate the last 8 bits of r9d, store in r9d
	movl %r9d, %r12d		//move green value in r9d into r12d
	movl %esi, %r9d			//save background value for later

	/*get a value of the foreground*/
	andl $255, %r8d			//isolate last 8 bits of foreground, store in r8d
	movl %r8d, %ecx			//move alpha value in r8d into ecx
	movl %edi, %r8d			//save foreground value for later

	/*call blend_components*/
	movl %r11d, %edi		//prepare to call blend_components
	movl %r12d, %esi		//prepare to call blend_components
	movl %ecx, %edx			//prepare to call blend_components
	call blend_components	//store blended g value in al
	movb %al, %r14b			//move blended g value to r14b, which is callee-saved
	movl %r8d, %edi			//move foreground color back into edi
	movl %r9d, %esi			//move background color back into esi

	/*get b component of the foreground*/
	shrl $8, %r8d			//move the 8 middle-right bits of foreground to the right, store in r8d
	andl $255, %r8d			//isolate the last 8 bits of r8d, store in r8d
	movl %r8d, %r11d		//move blue value in r8d into r11d
	movl %edi, %r8d			//save foreground value for later

	/*get b value of the background*/
	shrl $8, %r9d			//move the 8 middle-right bits of background to the right, store in r9d
	andl $255, %r9d			//isolate the last 8 bits of r9d, store in r9d
	movl %r9d, %r12d		//move blue value in r9d into r12d
	movl %esi, %r9d			//save background value for later

	/*get a value of the foreground*/
	andl $255, %r8d			//isolate last 8 bits of foreground, store in r8d
	movl %r8d, %ecx			//move alpha value in r8d into ecx
	movl %edi, %r8d			//save foreground value for later

	/*call blend_components*/
	movl %r11d, %edi		//prepare to call blend_components
	movl %r12d, %esi		//prepare to call blend_components
	movl %ecx, %edx			//prepare to call blend_components
	call blend_components	//store blended b value in al
	movb %al, %r15b			//move blended b value to r15b, which is callee-saved
	movl %r8d, %edi			//move foreground color back into edi
	movl %r9d, %esi			//move background color back into esi
	
	/* assuming that r is in r13b, g is in r14b, b is in r15b*/
	/* uint32_t final = (r << 24) + (g << 16) + (b << 8) + 255*/
	movzbl %r13b, %edi		//zero extension of r
	movzbl %r14b, %esi		//zero extension of g
	movzbl %r15b, %edx		//zero extension of b
	shll $24, %edi			// shift r to the left by 24
	shll $16, %esi			// shift g to the left by 16
	shll $8, %edx			// shift b to the left by 8
	mov $255, %eax			// prepare for addition by moving 255 to eax
	addl %edx, %eax			//add b and 255, store in eax
	addl %esi, %eax			//add g and (b+255), store in eax
	addl %edi, %eax			//add r and (g+b+255), store in eax
	
	// restore the values of callee-saved registers
	addq $8, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	ret

/*
 * Modifies a pixel in the background image to represent
 * the blended color value of the corresponding pixels in
 * the background image and the foreground image.
 * 
 * Parameters:
 *   %rdi     - pointer to the struct Image
 *   %esi     - the index of the pixel to modify
 *   %edx     - uint32_t color value
 */
	.globl set_pixel
set_pixel:
	pushq %r13
	pushq %r14
	subq $8, %rsp 					// aligns the stack pointer

	movl IMAGE_DATA_OFFSET(%rdi), %r13d		//get pointer to data array and put in r13d
	movl %esi, %r14d				//move index to r14d
	
<<<<<<< HEAD
	movl (%r13d,%r14d,4), %r8d				//retrieve the element value and put in %r8d
	movl %edx, %edi							//prepare color as a parameter				
	movl %r8d, %esi							//prepare background color as a parameter
=======
	movl (%r13d,%r14d,4), %r8d			//retrieve the element value and put in %r8
	movl %edx, %edi					//prepare color as a parameter				
	movl %r8d, %esi					//prepare background color as a parameter
>>>>>>> 9a916d9b7c78ddce0b4f5bcfafc65f905ddf7861
	
	call blend_colors				//store result in eax
	movl %eax, (%r13d,%r14d,4)			//move the blended color and put it back in the array

	addq $8, %rsp 					// aligns the stack pointer
	popq %r14
	popq %r13
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
	imulq %rdi, %rdi			//multiply rdi by rdi and store in rdi
	movq %rdi, %rax				//move product to rax for return
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
	subq $8, %rsp 		 // aligns the stack pointer
	
	movq %rsi, %r12 	 // save the value of y1 in r12
	movq %rcx, %r13 	 // save the value of y2 in r13
	
	subq %rdi, %rdx 	 // store x2-x1 in rdx
	movq %rdx, %rdi		 // prepare to call square
	call square			 // store (x2-x1)^2 in rax
	
	subq %r12, %r13 	 // store y1-y2 in r13
	movq %rax, %r12 	 // store (x2-x1)^2 in r12 now
	movq %r13, %rdi 	 // prepare to call square
	call square			 // store (y2-y1)^2 in rax
	
	addq %r12, %rax		 // store the square_dist in rax
	
	// restore the values of callee-saved registers
	addq $8, %rsp 		 // restore the stack pointer
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
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	subq $8, %rsp
 	// aligns the stack pointer

	movl %esi, %r12d
        movl %edx, %r13d
	movl %ecx, %r14d
	movq %rdi, %r15
	call in_bounds		// store in_bounds result in eax
	cmp $1, %eax		// compare 1 to eax
	je .LoutOfRange		// if in_bounds is 1, jump to LoutOfRange
	
	movl %r12d, %esi
        movl %r13d, %edx
	movq %r15, %rdi
	movl %esi, %r12d
        movl %edx, %r13d
	movq %rdi, %r15
	call compute_index	// store compute_index in eax
	movl %r14d, %esi	// move the color to esi
	movl %eax, %ecx		// move compute_index to ecx/rcx
	movq %r15, %rdi
	call set_pixel		// call set_pixel to finish off
	jmp .LendOff		// return eventually
.LoutOfRange:
	nop			// do nothing
.LendOff:
 	// restore the stack pointer
	addq $8, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
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
