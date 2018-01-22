static int alawsearch(int val)
{
	static short seg_end[8] = {0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};
	short *table = seg_end;
	int		i;

	for (i = 0; i < 8; i++) {
		if (val <= *table++)
			return (i);
	}
	return 8;
}

unsigned char linear2alaw(int pcm_val)
{
	int		mask;
	int		seg;
	unsigned char	aval;

	pcm_val /= 8;
	if (pcm_val >= 0) {
		mask = 0xD5;		/* sign (7th) bit = 1 */
	} else {
		mask = 0x55;		/* sign bit = 0 */
		pcm_val = -pcm_val - 1;
	}

	/* Convert the scaled magnitude to segment number. */
	seg = alawsearch(pcm_val);

	/* Combine the sign, segment, and quantization bits. */

	if (seg >= 8)		/* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		aval = seg << 4;
		if (seg < 2)
			aval |= (pcm_val >> 1) & 0xf;
		else
			aval |= (pcm_val >> seg) & 0xf;
		return (aval ^ mask);
	}
}

int alaw2linear(unsigned char a_val)
{
	int		t;
	int		seg;

	a_val ^= 0x55;

	t = (a_val & 0xf) << 4;
	seg = ((unsigned)a_val & 0x70) >> 4;
	switch (seg) {
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
	}
	return ((a_val & 0x80) ? t : -t);
}

#define	BIAS		(0x84)		/* Bias for linear code. */
#define CLIP            8159

static int mulawsearch(int val)
{
	static short seg_end[8] = {0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF};
	short *table = seg_end;
	int		i;

	for (i = 0; i < 8; i++) {
		if (val <= *table++)
			return (i);
	}
	return 8;
}

unsigned char linear2mulaw(int pcm_val)
{
	int		mask;
	int		seg;
	unsigned char	uval;

	pcm_val = pcm_val >> 2;

	/* u-law inverts all bits */
	/* Get the sign and the magnitude of the value. */
	if (pcm_val < 0) {
		pcm_val = -pcm_val;
		mask = 0x7F;
	} else {
		mask = 0xFF;
	}
	if ( pcm_val > CLIP ) pcm_val = CLIP;		/* clip the magnitude */
	pcm_val += (BIAS >> 2);

	/* Convert the scaled magnitude to segment number. */
	seg = mulawsearch(pcm_val);

	/*
	 * Combine the sign, segment, quantization bits;
	 * and complement the code word.
	 */
	if (seg >= 8)		/* out of range, return maximum value. */
		return (unsigned char) (0x7F ^ mask);
	else {
		uval = (unsigned char) (seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
		return (uval ^ mask);
	}

}

#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */

int mulaw2linear(unsigned char u_val)
{
	int		t;

	/* Complement to obtain normal u-law value. */
	u_val = ~u_val;

	/*
	 * Extract and bias the quantization bits. Then
	 * shift up by the segment number and subtract out the bias.
	 */
	t = ((u_val & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

	return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}
